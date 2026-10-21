#include "QuestParty.h"
#include "Config.h"
#include "GameTime.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Log.h"
#include "Loot.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "../Services.h"
#include "../Bot/BotRegistry.h"
#include "OwnerRegistry.h"
#include "BotGroupBuilder.h"
#include "BotCoordinationBus.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Playerbot::V2::QuestParty {

namespace {

constexpr uint32 kTickMs          = 20u * 1000u;
constexpr float  kFormRange       = 40.0f;          // members meet where they stand
constexpr uint8  kLevelSpread     = 2;              // at forming
constexpr uint8  kLevelSpreadMax  = 3;              // a party whose levels drift further apart breaks up
constexpr float  kApartRange      = 150.0f;
constexpr uint32 kApartMs         = 3u * 60u * 1000u;
constexpr uint32 kCooldownMs      = 10u * 60u * 1000u;   // after a party, before the next one
constexpr uint32 kMoodWindowMs    = 30u * 60u * 1000u;   // who wants a party is re-rolled every 30 minutes

struct Party
{
    ObjectGuid group;
    uint32 ends_ms = 0;
    uint32 apart_since_ms = 0;
};

std::vector<Party> g_parties;                        // world thread
std::unordered_map<uint64, uint32> g_cooldown;       // world thread: bot -> no new party before
std::mutex g_members_mtx;
std::unordered_set<uint64> g_members;                // read by the bot workers

uint32 Share()   { return uint32(std::clamp(sConfigMgr->GetIntDefault("Playerbot.World.QuestPartyShare", 35, true), 0, 100)); }
uint32 MaxSize() { return uint32(std::clamp(sConfigMgr->GetIntDefault("Playerbot.World.QuestPartyMaxSize", 3, true), 2, 5)); }

uint32 Mix(uint64 a, uint32 b) { return uint32(a * 2654435761u) ^ (b * 40503u + 0x9e37u); }

Language Tongue(Player const* p) { return p->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH; }

bool IsWorldBot(uint64 low)
{
    if (!Services::Registry().has(low)) return false;
    auto const o = Services::Owners().GetOwner(low);
    return o.account_id == 0 && o.player_guid == 0;
}

bool InOpenWorld(Player const* p)
{
    return p->IsInWorld() && !p->IsBeingTeleported() && p->GetMap() && !p->GetMap()->Instanceable();
}

std::vector<uint32> OpenQuests(Player const* p)
{
    std::vector<uint32> out;
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        if (uint32 const q = p->GetQuestSlotQuestId(slot))
            if (p->GetQuestStatus(q) == QUEST_STATUS_INCOMPLETE)
                out.push_back(q);
    return out;
}

uint32 SharedQuest(std::vector<uint32> const& a, std::vector<uint32> const& b)
{
    for (uint32 q : a)
        if (std::find(b.begin(), b.end(), q) != b.end())
            return q;
    return 0;
}

std::vector<Player*> Members(Group* g)
{
    std::vector<Player*> out;
    for (Group::MemberSlot const& slot : g->GetMemberSlots())
        if (Player* p = ObjectAccessor::FindConnectedPlayer(slot.guid))
            out.push_back(p);
    return out;
}

void Publish()
{
    std::unordered_set<uint64> now;
    for (Party const& party : g_parties)
        if (Group* g = sGroupMgr->GetGroupByGUID(party.group))
            for (Group::MemberSlot const& slot : g->GetMemberSlots())
                now.insert(slot.guid.GetCounter());
    std::lock_guard lk(g_members_mtx);
    g_members.swap(now);
}

void Cool(Group* g, uint32 now)
{
    for (Group::MemberSlot const& slot : g->GetMemberSlots())
        g_cooldown[slot.guid.GetCounter()] = now + kCooldownMs;
}

void Disband(Group* g, uint32 now, char const* why)
{
    std::vector<Player*> const ms = Members(g);
    std::string names;
    for (Player* p : ms)
        names += (names.empty() ? "" : ", ") + p->GetName();
    if (!ms.empty() && ms.front()->IsAlive())
        ms.front()->Say(std::string_view(std::strcmp(why, "time") == 0 ? "Thanks for the group, good luck!" : "I'll go on alone from here, thanks!"),
            Tongue(ms.front()));
    TC_LOG_INFO("playerbot.v2", "[quest_party] broke up ({}): {}", why, names);
    Cool(g, now);
    g->Disband();
}

// Adopt bot-only open-world parties the matchmaker does not know (a party saved before a restart).
void Adopt(Player* p, uint32 now)
{
    Group* g = p->GetGroup();
    if (!g || g->isRaidGroup() || g->isLFGGroup() || g->isBGGroup() || g->isBFGroup()) return;
    if (std::any_of(g_parties.begin(), g_parties.end(), [g](Party const& x) { return x.group == g->GetGUID(); })) return;
    for (Group::MemberSlot const& slot : g->GetMemberSlots())
        if (!IsWorldBot(slot.guid.GetCounter())) return;
    g_parties.push_back({ g->GetGUID(), now + kMoodWindowMs, 0 });
    TC_LOG_INFO("playerbot.v2", "[quest_party] adopted a saved party of {} led by {}", g->GetMembersCount(), p->GetName());
}

void Maintain(uint32 now)
{
    for (auto it = g_parties.begin(); it != g_parties.end();)
    {
        Group* g = sGroupMgr->GetGroupByGUID(it->group);
        if (!g)
        {
            it = g_parties.erase(it);
            continue;
        }
        bool human = false;
        for (Group::MemberSlot const& slot : g->GetMemberSlots())
            if (!IsWorldBot(slot.guid.GetCounter())) human = true;
        if (human)
        {
            // A real player (or someone's companion) joined: from now on it is his group, the matchmaker lets go of it.
            TC_LOG_INFO("playerbot.v2", "[quest_party] a player joined the party led by {}, released", g->GetLeaderName());
            it = g_parties.erase(it);
            continue;
        }
        std::vector<Player*> const ms = Members(g);
        char const* why = nullptr;
        if (g->GetMembersCount() < 2 || ms.size() < 2)
            why = "too few";
        else if (now >= it->ends_ms)
            why = "time";
        else
        {
            uint8 lo = 255, hi = 0;
            bool apart = false, instance = false;
            for (Player* p : ms)
            {
                lo = std::min(lo, p->GetLevel());
                hi = std::max(hi, p->GetLevel());
                if (!p->IsBeingTeleported() && !InOpenWorld(p)) instance = true;   // a teleport (death-loop rescue) is not an instance
                if (p->GetMapId() != ms.front()->GetMapId() || p->GetExactDist2d(ms.front()) > kApartRange) apart = true;
            }
            if (hi - lo > kLevelSpreadMax)
                why = "levels";
            else if (instance)
                why = "instance";
            else if (apart)
            {
                if (!it->apart_since_ms)
                    it->apart_since_ms = now ? now : 1u;
                else if (now - it->apart_since_ms >= kApartMs)
                    why = "apart";
            }
            else
                it->apart_since_ms = 0;
        }
        if (why)
        {
            Disband(g, now, why);
            it = g_parties.erase(it);
            continue;
        }
        ++it;
    }
}

struct Candidate
{
    Player* p;
    std::vector<uint32> quests;
    bool used = false;
};

void Form(uint32 now)
{
    std::vector<uint64> ids;
    Services::Registry().for_each([&](BotId id, auto const&) { ids.push_back(id); });
    uint32 const share = Share();
    std::vector<Candidate> cands;
    for (uint64 id : ids)
    {
        Player* p = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(id));
        if (!p || !InOpenWorld(p) || !p->IsAlive() || p->IsInCombat() || p->IsInFlight()) continue;
        if (p->GetGroup())
        {
            Adopt(p, now);
            continue;
        }
        if (!IsWorldBot(id)) continue;
        if (auto c = g_cooldown.find(id); c != g_cooldown.end() && now < c->second) continue;
        if (Mix(id, now / kMoodWindowMs) % 100u >= share) continue;   // not in the mood for company right now
        std::vector<uint32> q = OpenQuests(p);
        if (q.empty()) continue;
        cands.push_back({ p, std::move(q) });
    }

    uint32 const max_size = MaxSize();
    for (size_t i = 0; i < cands.size(); ++i)
    {
        Candidate& a = cands[i];
        if (a.used) continue;
        std::vector<Candidate*> mates;
        uint32 quest = 0;
        for (size_t j = 0; j < cands.size() && mates.size() + 1 < max_size; ++j)
        {
            Candidate& b = cands[j];
            if (j == i || b.used) continue;
            if (b.p->GetTeam() != a.p->GetTeam() || b.p->GetMapId() != a.p->GetMapId()) continue;
            if (std::abs(int32(b.p->GetLevel()) - int32(a.p->GetLevel())) > kLevelSpread) continue;
            if (b.p->GetExactDist2d(a.p) > kFormRange) continue;
            uint32 const q = SharedQuest(a.quests, b.quests);
            if (!q || (quest && q != quest)) continue;
            quest = q;
            mates.push_back(&b);
        }
        if (mates.empty()) continue;

        Group* g = new Group();
        if (!g->Create(a.p))
        {
            delete g;
            continue;
        }
        sGroupMgr->AddGroup(g);
        g->SetLootMethod(FREE_FOR_ALL);   // nobody waits on a roll; every member loots its own kills
        std::string names = a.p->GetName();
        for (Candidate* m : mates)
            if (g->AddMember(m->p))
            {
                m->used = true;
                names += ", " + m->p->GetName();
            }
        if (g->GetMembersCount() < 2)
        {
            g->Disband();
            continue;
        }
        a.used = true;
        g->BroadcastGroupUpdate();
        uint32 const minutes = 30u + Mix(a.p->GetGUID().GetCounter(), now / 1000u) % 31u;
        g_parties.push_back({ g->GetGUID(), now + minutes * 60u * 1000u, 0 });

        Quest const* qt = sObjectMgr->GetQuestTemplate(quest);
        std::string const title = qt ? std::string(qt->GetLogTitle()) : "this";
        a.p->Say("Doing " + title + " too? Let's group up.", Tongue(a.p));
        mates.front()->p->Say(std::string_view("Sure, let's go."), Tongue(mates.front()->p));
        TC_LOG_INFO("playerbot.v2", "[quest_party] formed: {} (L{}, map {} zone {}) for quest {} \"{}\", {} min", names,
            uint32(a.p->GetLevel()), a.p->GetMapId(), a.p->GetZoneId(), quest, title, minutes);
    }
}

// ---- Dungeon groups (Playerbot.World.DungeonGroups) ----
// Upstream forms bot dungeon groups only for guild dungeon nights (Tuesday and Thursday 20:00, guild members of 15+). Every
// Playerbot.World.DungeonGroupMinutes (20) per faction, when at least 8 free world bots of 15+ are online, this asks the same
// builder for one group of 1 tank, 1 healer and 3 damage (BotGroupBuilder::BuildGroup, source GuildEventForming, content 3 =
// dungeon night, any guild): ~10 s later its leader queues for a dungeon of its level and the dungeon autopilot runs it.
uint32 g_last_dungeon_ms[2] = { 0, 0 };

void DriveDungeonGroups(uint32 now)
{
    static bool const enabled = sConfigMgr->GetBoolDefault("Playerbot.World.DungeonGroups", false, true);
    static uint32 const every = uint32(std::max(5, sConfigMgr->GetIntDefault("Playerbot.World.DungeonGroupMinutes", 20, true))) * 60000u;
    constexpr uint8 kMinLevel = 15;     // Ragefire Chasm, the lowest bracket the builder's dungeon pick queues for
    constexpr uint32 kMinFree = 8;      // enough bots that a 5-man with a tank and a healer can be found
    if (!enabled)
        return;
    std::vector<uint64> ids;
    Services::Registry().for_each([&](BotId id, auto const&) { ids.push_back(id); });
    uint32 free_count[2] = { 0, 0 };
    uint8 lo[2] = { 255, 255 }, hi[2] = { 0, 0 };
    for (uint64 id : ids)
    {
        Player* p = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(id));
        if (!p || !InOpenWorld(p) || !p->IsAlive() || p->GetGroup() || p->GetLevel() < kMinLevel || !IsWorldBot(id))
            continue;
        uint8 const f = p->GetTeam() == ALLIANCE ? 0 : 1;
        ++free_count[f];
        lo[f] = std::min(lo[f], p->GetLevel());
        hi[f] = std::max(hi[f], p->GetLevel());
    }
    for (uint8 f = 0; f < 2; ++f)
    {
        if (free_count[f] < kMinFree || (g_last_dungeon_ms[f] && now - g_last_dungeon_ms[f] < every))
            continue;
        g_last_dungeon_ms[f] = now ? now : 1u;
        ::Playerbot::V2::GroupRequest req{};
        req.source = ::Playerbot::V2::CoordSignal::GuildEventForming;
        req.content_id = 3;   // dungeon night: the builder's follow-up queues the leader for a dungeon of its level
        req.required = 5;
        req.want_tank = 1;
        req.want_healer = 1;
        req.want_dps = 3;
        req.faction_mask = f == 0 ? 1u : 2u;
        req.level_min = kMinLevel;
        req.level_max = std::min<uint8>(uint8(hi[f]), uint8(kMinLevel + 9));
        uint32 const invited = Services::GroupBuilder().BuildGroup(req);
        TC_LOG_INFO("playerbot.v2", "[dungeon_group] {}: {} free bots of {}+ (levels {}-{}), builder invited {}", f == 0 ? "Alliance" : "Horde",
            free_count[f], uint32(kMinLevel), uint32(lo[f]), uint32(hi[f]), invited);
    }
}

} // namespace

bool Enabled() { return sConfigMgr->GetBoolDefault("Playerbot.World.QuestParties", false, true); }

void Drive(uint32 now_ms)
{
    static uint32 last = 0;
    if (now_ms - last < kTickMs) return;
    last = now_ms;
    if (!Services::Initialized())
        return;
    DriveDungeonGroups(now_ms);
    if (!Enabled())
        return;
    Maintain(now_ms);
    Form(now_ms);
    std::erase_if(g_cooldown, [now_ms](auto const& kv) { return now_ms >= kv.second; });
    Publish();
}

bool IsMember(uint64 bot_low)
{
    std::lock_guard lk(g_members_mtx);
    return g_members.contains(bot_low);
}

} // namespace Playerbot::V2::QuestParty
