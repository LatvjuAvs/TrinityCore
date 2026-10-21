// QuestPartyRules - local (WoW Forever local server, M7 "living world"): idle:party_regroup keeps a quest party
// (Fleet/QuestParty.h, Playerbot.World.QuestParties) together. A member more than 45 y from its leader, out of combat, walks back
// until it is within 12 y; otherwise every member quests with its own rules, which for bots on the same quest means the same
// mob camp. Registered from RegisterTrainerRules so IdleRule.h (included everywhere) stays unchanged.

#include "Bot/IdleRule.h"
#include "Group/GroupSnapshot.h"
#include "Bot/BotAI.h"
#include "Bot/BotSnapshotView.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/States/MaintainHelpers.h"
#include "Fleet/QuestParty.h"
#include "GameTime.h"
#include <mutex>
#include <unordered_map>
#include "Log.h"
#include <cmath>
#include <cstring>

namespace Playerbot {

namespace {

constexpr float kRegroupFar  = 45.0f;
constexpr float kRegroupNear = 12.0f;

GroupMemberSummary const* PartyLeader(BotSnapshotView const& s, GroupSnapshotView const& g)
{
    if (!g.exists() || g.leader() == s.raw().guid) return nullptr;
    if (auto const* ms = g.members())
        for (auto const& m : *ms)
            if (m.guid == g.leader())
                return &m;
    return nullptr;
}

bool PartyRegroupGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const& g, uint32)
{
    if (!s.is_alive() || s.in_combat() || s.is_casting() || s.is_in_instance() || s.in_battleground()) return false;
    if (!g.exists() || !V2::QuestParty::IsMember(s.bot_id())) return false;
    GroupMemberSummary const* lead = PartyLeader(s, g);
    if (!lead || lead->map_id != s.map_id() || lead->hp <= 0) return false;
    float bx, by, bz; s.position(bx, by, bz);
    float const d = std::hypot(lead->x - bx, lead->y - by);
    bool const regrouping = ai.last_rule_fired() && std::strcmp(ai.last_rule_fired(), "idle:party_regroup") == 0;
    return d > kRegroupFar || (regrouping && d > kRegroupNear);
}

bool PartyRegroupFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const& g, BotIntentEmitter& emit, uint32)
{
    GroupMemberSummary const* lead = PartyLeader(s, g);
    if (!lead) return false;
    float bx, by, bz; s.position(bx, by, bz);
    float const d = std::hypot(lead->x - bx, lead->y - by);
    if (d <= 60.0f)
    {
        if (emit.move_to(lead->x, lead->y, lead->z, /*run*/ true) || emit.move_already_emitted(lead->x, lead->y, lead->z))
        {
            ai.set_last_rule_fired("idle:party_regroup");
            return true;
        }
    }
    else if (States::DriveTravelPlanTo(s, ai, emit, bx, by, bz, lead->map_id, lead->x, lead->y, lead->z, /*quest_goal=*/false))
    {
        ai.set_last_rule_fired("idle:party_regroup");
        return true;
    }
    return States::ChunkedWalkToward(s, ai, emit, lead->x, lead->y, lead->z, "idle:party_regroup", GameTime::GetGameTimeMS());
}

} // namespace

namespace {
// Local (M7): idle:stranded_hearth. A low-level bot with no quest objective while its hearthstone points to the other
// continent has been carried away from its quests (charter signer pulls before 2026-10-21 20:23; the picker never chooses an
// incomplete objective across continents, so [picker_none] Stalen stood in Teldrassil with only Dun Morogh quests). After
// 2 minutes without an objective it hearths home, at most once per 30 minutes.
std::mutex g_stranded_mtx;
std::unordered_map<uint64, std::pair<uint32, uint32>> g_stranded;   // bot -> (no objective since ms, last hearth ms)

bool StrandedHearthGate(BotSnapshotView const& s, BotAI&, GroupSnapshotView const&, uint32)
{
    if (s.level() >= 20 || s.in_combat() || s.raw().movement.is_mounted || s.has_owner_character()) return false;
    if (s.in_group() || s.is_in_dungeon() || s.in_battleground() || s.is_in_instance() || s.on_transport()) return false;
    if (s.homebind_map_id() == s.map_id()) return false;   // cheap test first: only a bot away from its home map qualifies
    uint32 const now = s.published_at_ms();
    std::lock_guard<std::mutex> lk(g_stranded_mtx);
    // The hearth is a 10 s cast that any move interrupts: hold the bot still for 12 s after starting it (live 2026-10-21:
    // two night elves in Stormwind walked off and their hearths failed).
    if (auto it = g_stranded.find(s.bot_id()); it != g_stranded.end() && it->second.second && now - it->second.second < 12000u &&
        s.homebind_map_id() != s.map_id())
        return true;
    if (s.is_casting()) return false;
    if (s.raw().quest_log.current_quest_id == 0 && s.raw().quest_log.quests.empty()) return false;
    if (s.raw().quest_log.current_quest_id != 0 || s.raw().quest_log.quests.empty())
    {
        if (auto it = g_stranded.find(s.bot_id()); it != g_stranded.end()) it->second.first = 0;   // keep the last hearth time
        return false;
    }
    if (!s.can_hearth() || s.homebind_map_id() == s.map_id()) return false;
    auto& e = g_stranded[s.bot_id()];
    if (e.first == 0) e.first = now ? now : 1u;
    if (now - e.first < 2u * 60u * 1000u) return false;
    return e.second == 0 || now - e.second > 30u * 60u * 1000u;
}

bool StrandedHearthFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    {
        std::lock_guard<std::mutex> lk(g_stranded_mtx);
        if (auto it = g_stranded.find(s.bot_id()); it != g_stranded.end() && it->second.second &&
            s.published_at_ms() - it->second.second < 12000u)
        {
            ai.set_last_rule_fired("idle:stranded_hearth:casting");
            return true;   // stand still while the hearth casts
        }
    }
    if (!emit.hearth()) return false;
    {
        std::lock_guard<std::mutex> lk(g_stranded_mtx);
        auto& e = g_stranded[s.bot_id()];
        e.second = s.published_at_ms() ? s.published_at_ms() : 1u;
        e.first = 0;
    }
    TC_LOG_INFO("playerbot.v2", "[stranded_hearth] {} L{} has no quest objective on map {} and hearths home to map {}",
        s.name(), uint32(s.level()), s.map_id(), s.homebind_map_id());
    ai.set_last_rule_fired("idle:stranded_hearth");
    return true;
}
} // namespace

void RegisterQuestPartyRules(IdleRuleRegistry& r)
{
    {
        IdleRule hr;
        hr.name     = "idle:stranded_hearth";
        hr.priority = 699;   // just above idle:pursue_quest_goal (698), which has nothing to pursue here
        hr.gate     = &StrandedHearthGate;
        hr.fire     = &StrandedHearthFire;
        hr.min_interval_ms = 0;   // it holds the bot still while the hearth casts (a throttle lets other rules move it)
        r.register_rule(std::move(hr));
    }
    IdleRule rule;
    rule.name     = "idle:party_regroup";
    rule.priority = 235;   // below the class trainer trip (240), above idle:trainer (230) and the quest work in autoact (50)
    rule.gate     = &PartyRegroupGate;
    rule.fire     = &PartyRegroupFire;
    r.register_rule(std::move(rule));
}

} // namespace Playerbot
