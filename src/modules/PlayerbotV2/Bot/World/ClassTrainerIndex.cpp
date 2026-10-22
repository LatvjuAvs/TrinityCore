#include "ClassTrainerIndex.h"
#include "Config.h"
#include "DB2Stores.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Map.h"
#include "Trainer.h"
#include "GameTime.h"
#include "Services.h"
#include "Bot/BotRegistry.h"
#include "Fleet/OwnerRegistry.h"
#include <cmath>
#include <mutex>
#include <algorithm>
#include <array>
#include <unordered_map>

namespace Playerbot::World {

namespace {

struct TrainerSpellRow
{
    uint32 spell = 0;
    uint8  req_level = 0;
    std::array<uint32, 3> req_ability = {};
};

struct Index
{
    bool built = false;
    // class id -> spots of trainers that teach that class
    std::unordered_map<uint8, std::vector<ClassTrainerSpot>> spots;
    // class id -> spells its class trainers teach (deduplicated, lowest level)
    std::unordered_map<uint8, std::vector<TrainerSpellRow>> spells;
    // creature entry -> faction template, to keep only trainers friendly to the bot
    std::unordered_map<uint32, uint32> faction;
};

Index& TheIndex() { static Index idx; return idx; }

// Class whose spell family the trainer teaches (ChrClasses::SpellClassSet), 0 when it teaches no class spells.
uint8 ClassOfFamily(uint32 family)
{
    if (!family)
        return 0;
    for (ChrClassesEntry const* cls : sChrClassesStore)
        if (cls && cls->SpellClassSet == family)
            return uint8(cls->ID);
    return 0;
}

void Build()
{
    Index& idx = TheIndex();
    idx.built = true;

    // Class trainers (trainer Type 0) and their spells: a trainer belongs to the class of its first class-family spell.
    std::unordered_map<uint32, uint8> trainer_class;
    if (QueryResult r = WorldDatabase.Query(
            "SELECT ts.TrainerId, ts.SpellId, ts.ReqLevel, ts.ReqAbility1, ts.ReqAbility2, ts.ReqAbility3 "
            "FROM trainer_spell ts JOIN trainer t ON t.Id = ts.TrainerId WHERE t.Type = 0"))
    {
        std::unordered_map<uint8, std::unordered_map<uint32, TrainerSpellRow>> by_class;
        std::vector<std::pair<uint32, TrainerSpellRow>> rows;
        do
        {
            Field* f = r->Fetch();
            uint32 const trainer = f[0].GetUInt32();
            TrainerSpellRow row;
            row.spell = f[1].GetUInt32();
            row.req_level = f[2].GetUInt8();
            for (uint8 i = 0; i < 3; ++i)
                row.req_ability[i] = f[3 + i].GetUInt32();
            SpellInfo const* info = sSpellMgr->GetSpellInfo(row.spell, DIFFICULTY_NONE);
            if (!info)
                continue;
            if (!trainer_class.contains(trainer))
                if (uint8 cls = ClassOfFamily(info->SpellFamilyName))
                    trainer_class[trainer] = cls;
            rows.emplace_back(trainer, row);
        } while (r->NextRow());

        for (auto const& [trainer, row] : rows)
        {
            auto it = trainer_class.find(trainer);
            if (it == trainer_class.end())
                continue;
            auto& slot = by_class[it->second];
            auto [pos, inserted] = slot.try_emplace(row.spell, row);
            if (!inserted && row.req_level < pos->second.req_level)
                pos->second = row;
        }
        for (auto& [cls, m] : by_class)
            for (auto& [spell, row] : m)
                idx.spells[cls].push_back(row);
    }

    // Where those trainers stand.
    if (QueryResult r = WorldDatabase.Query(
            "SELECT c.map, c.position_x, c.position_y, c.position_z, ct.entry, ct.faction, ctr.TrainerID "
            "FROM creature c JOIN creature_template ct ON ct.entry = c.id JOIN creature_trainer ctr ON ctr.CreatureID = ct.entry"))
    {
        do
        {
            Field* f = r->Fetch();
            auto it = trainer_class.find(f[6].GetUInt32());
            if (it == trainer_class.end())
                continue;
            ClassTrainerSpot spot;
            spot.map = f[0].GetUInt16();
            spot.x = f[1].GetFloat();
            spot.y = f[2].GetFloat();
            spot.z = f[3].GetFloat();
            spot.creature_entry = f[4].GetUInt32();
            spot.trainer_id = f[6].GetUInt32();
            idx.faction[spot.creature_entry] = f[5].GetUInt16();
            idx.spots[it->second].push_back(spot);
        } while (r->NextRow());
    }

    size_t n_spots = 0, n_spells = 0;
    for (auto const& [c, v] : idx.spots) n_spots += v.size();
    for (auto const& [c, v] : idx.spells) n_spells += v.size();
    TC_LOG_INFO("playerbot.v2", "[earn_spells] class trainer index: {} trainers, {} trainer spells over {} classes", n_spots, n_spells,
        idx.spells.size());
}

// Built once, at the first level-up; OnLevelUp runs on a map thread when MapUpdate.Threads > 1. Read-only afterwards.
Index& Built()
{
    static std::once_flag once;
    std::call_once(once, [] { Build(); });
    return TheIndex();
}

} // namespace

bool EarnSpellsEnabled()
{
    return sConfigMgr->GetBoolDefault("Playerbot.Bot.EarnSpells", false, true);
}

uint32 CountLearnableTrainerSpells(Player* bot)
{
    Index& idx = Built();
    auto it = idx.spells.find(bot->GetClass());
    if (it == idx.spells.end())
        return 0;
    uint32 n = 0;
    uint8 const level = bot->GetLevel();
    for (TrainerSpellRow const& row : it->second)
    {
        if (row.req_level > level || bot->HasSpell(row.spell))
            continue;
        if (std::ranges::any_of(row.req_ability, [bot](uint32 a) { return a && !bot->HasSpell(a); }))
            continue;
        if (!bot->IsSpellFitByClassAndRace(row.spell))
            continue;
        ++n;
    }
    return n;
}

bool NearestClassTrainer(Player* bot, float max_dist, ClassTrainerSpot& out)
{
    Index& idx = Built();
    auto it = idx.spots.find(bot->GetClass());
    if (it == idx.spots.end())
        return false;
    FactionTemplateEntry const* mine = bot->GetFactionTemplateEntry();
    float best = max_dist * max_dist;
    bool found = false;
    for (ClassTrainerSpot const& spot : it->second)
    {
        if (spot.map != bot->GetMapId())
            continue;
        if (mine)
            if (FactionTemplateEntry const* theirs = sFactionTemplateStore.LookupEntry(idx.faction[spot.creature_entry]))
                if (!mine->IsFriendlyTo(theirs) && !theirs->IsFriendlyTo(mine))
                    continue;
        float const dx = spot.x - bot->GetPositionX(), dy = spot.y - bot->GetPositionY();
        float const d2 = dx * dx + dy * dy;
        if (d2 < best)
        {
            best = d2;
            out = spot;
            found = true;
        }
    }
    return found;
}

namespace {
std::mutex g_remote_mtx;
std::vector<uint64> g_remote;
}

void QueueRemoteTraining(uint64 bot_low)
{
    std::lock_guard lk(g_remote_mtx);
    g_remote.push_back(bot_low);
}

void DriveRemoteTraining()
{
    std::vector<uint64> todo;
    {
        std::lock_guard lk(g_remote_mtx);
        todo.swap(g_remote);
    }
    for (uint64 low : todo)
    {
        Player* bot = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(low));
        if (!bot || !bot->IsInWorld())
            continue;
        ClassTrainerSpot spot;
        if (!NearestClassTrainer(bot, 5000.0f, spot))
            continue;
        Trainer::Trainer const* trainer = sObjectMgr->GetTrainer(spot.trainer_id);
        if (!trainer)
            continue;
        std::vector<Trainer::Spell const*> can;
        for (Trainer::Spell const& sp : trainer->GetSpells())
        {
            if (sp.ReqSkillLine || sp.ReqLevel > bot->GetLevel() || bot->HasSpell(sp.SpellId) || !bot->IsSpellFitByClassAndRace(sp.SpellId))
                continue;
            if (std::ranges::any_of(sp.ReqAbility, [bot](uint32 a) { return a && !bot->HasSpell(a); }))
                continue;
            can.push_back(&sp);
        }
        std::ranges::sort(can, {}, [](Trainer::Spell const* sp) { return sp->MoneyCost; });
        uint32 learned = 0;
        uint64 paid = 0;
        for (Trainer::Spell const* sp : can)
        {
            if (!bot->HasEnoughMoney(int64(sp->MoneyCost)))
                break;
            bot->ModifyMoney(-int64(sp->MoneyCost));
            if (sp->IsCastable())
                bot->CastSpell(bot, sp->SpellId, true);
            else
                bot->LearnSpell(sp->SpellId, false);
            ++learned;
            paid += sp->MoneyCost;
        }
        TC_LOG_INFO("playerbot.v2", "[earn_spells] {} L{} could not reach trainer {}: bought {} of {} spell(s) for {} copper ({} left)",
            bot->GetName(), uint32(bot->GetLevel()), spot.creature_entry, learned, can.size(), paid, bot->GetMoney());
    }
}

// ---------- Local (M7): professions ----------
namespace {
struct Prof { uint32 spell; uint16 skill; char const* name; };
constexpr Prof kHerb{ 2366, 182, "Herbalism" }, kMine{ 2575, 186, "Mining" }, kSkin{ 8613, 393, "Skinning" },
               kAlch{ 2259, 171, "Alchemy" }, kSmith{ 2018, 164, "Blacksmithing" }, kLeather{ 2108, 165, "Leatherworking" },
               kTailor{ 3908, 197, "Tailoring" }, kEngi{ 4036, 202, "Engineering" }, kEnch{ 7411, 333, "Enchanting" };

// The two professions a bot of this class goes for (the first a gathering one), chosen once by its guid.
std::pair<Prof, Prof> ProfessionPair(uint8 cls, uint64 guid)
{
    using P = std::pair<Prof, Prof>;
    static std::unordered_map<uint8, std::vector<P>> const choices = {
        { CLASS_WARRIOR, { { kMine, kSmith }, { kMine, kEngi }, { kMine, kSkin } } },
        { CLASS_PALADIN, { { kMine, kSmith }, { kHerb, kMine } } },
        { CLASS_HUNTER,  { { kSkin, kLeather }, { kSkin, kMine } } },
        { CLASS_ROGUE,   { { kSkin, kLeather }, { kHerb, kAlch }, { kMine, kEngi } } },
        { CLASS_PRIEST,  { { kHerb, kAlch }, { kTailor, kEnch } } },
        { CLASS_MAGE,    { { kTailor, kEnch }, { kHerb, kAlch } } },
        { CLASS_WARLOCK, { { kTailor, kEnch }, { kHerb, kAlch } } },
        { CLASS_DRUID,   { { kHerb, kSkin }, { kSkin, kLeather }, { kHerb, kAlch } } },
        { CLASS_SHAMAN,  { { kHerb, kAlch }, { kMine, kSkin }, { kSkin, kLeather } } },
    };
    auto it = choices.find(cls);
    if (it == choices.end() || it->second.empty())
        return { kHerb, kAlch };
    return it->second[(guid * 2654435761ull >> 7) % it->second.size()];
}

struct ProfSpot { uint32 map; float x, y, z; uint32 entry; uint32 faction; };
std::unordered_map<uint32, std::vector<ProfSpot>> const& ProfessionTrainers()
{
    static std::unordered_map<uint32, std::vector<ProfSpot>> spots;
    static std::once_flag once;
    std::call_once(once, []
    {
        if (QueryResult r = WorldDatabase.Query(
                "SELECT ts.SpellId, c.map, c.position_x, c.position_y, c.position_z, ct.entry, ct.faction "
                "FROM trainer_spell ts JOIN trainer t ON t.Id = ts.TrainerId AND t.Type = 2 "
                "JOIN creature_trainer ctr ON ctr.TrainerID = t.Id JOIN creature_template ct ON ct.entry = ctr.CreatureID "
                "JOIN creature c ON c.id = ct.entry "
                "WHERE ts.SpellId IN (2366, 2575, 8613, 2259, 2018, 2108, 3908, 4036, 7411)"))
            do
            {
                Field* f = r->Fetch();
                spots[f[0].GetUInt32()].push_back({ f[1].GetUInt16(), f[2].GetFloat(), f[3].GetFloat(), f[4].GetFloat(),
                                                    f[5].GetUInt32(), f[6].GetUInt16() });
            } while (r->NextRow());
        size_t n = 0;
        for (auto const& [s, v] : spots) n += v.size();
        TC_LOG_INFO("playerbot.v2", "[profession] trainer index: {} spawns for {} Apprentice professions", n, spots.size());
    });
    return spots;
}

std::mutex g_prof_mtx;
std::unordered_map<uint64, ProfessionTrip> g_prof_trip;
std::unordered_map<uint64, uint32> g_prof_retry_after;   // bot -> game ms before which no new trip is planned
} // namespace

bool ProfessionTripFor(uint64 bot_low, ProfessionTrip& out)
{
    std::lock_guard lk(g_prof_mtx);
    auto it = g_prof_trip.find(bot_low);
    if (it == g_prof_trip.end() || !it->second.active)
        return false;
    out = it->second;
    return true;
}

void EndProfessionTrip(uint64 bot_low, uint32 retry_after_ms)
{
    std::lock_guard lk(g_prof_mtx);
    g_prof_trip.erase(bot_low);
    if (retry_after_ms)
        g_prof_retry_after[bot_low] = GameTime::GetGameTimeMS() + retry_after_ms;
}

void PlanProfessionTrips(uint32 now_ms)
{
    static bool const enabled = sConfigMgr->GetBoolDefault("Playerbot.Bot.LearnProfessions", true, true);
    static uint32 s_last = 0;
    if (!enabled || (s_last && now_ms - s_last < 20000u))
        return;
    s_last = now_ms ? now_ms : 1u;
    auto const& trainers = ProfessionTrainers();
    std::vector<uint64> ids;
    Services::Registry().for_each([&](auto id, auto const&) { ids.push_back(uint64(id)); });
    uint32 planned = 0;
    for (uint64 id : ids)
    {
        Player* bot = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(id));
        if (!bot || !bot->IsInWorld() || !bot->IsAlive() || bot->GetLevel() < 5 || bot->IsInCombat() || bot->GetGroup() ||
            bot->GetMap()->Instanceable() || bot->GetMoney() < 10)
            continue;
        if (Services::Owners().GetOwner(id).account_id != 0)
            continue;   // a player's companion
        {
            std::lock_guard lk(g_prof_mtx);
            if (g_prof_trip.contains(id))
                continue;
            if (auto r = g_prof_retry_after.find(id); r != g_prof_retry_after.end() && int32(r->second - now_ms) > 0)
                continue;
        }
        // Two primary professions at most: bots set up at 10+ before FreshStart already hold two (granted by the setup).
        static constexpr uint16 kPrimary[] = { 182, 186, 393, 171, 164, 165, 197, 202, 333 };
        uint32 primaries = 0;
        for (uint16 sk : kPrimary)
            if (bot->HasSkill(sk)) ++primaries;
        if (primaries >= 2)
            continue;
        auto const [a, b] = ProfessionPair(bot->GetClass(), id);
        Prof const* want = nullptr;
        for (Prof const* p : { &a, &b })
            if (!bot->HasSkill(p->skill) && !bot->HasSpell(p->spell)) { want = p; break; }
        if (!want)
            continue;
        auto it = trainers.find(want->spell);
        if (it == trainers.end())
            continue;
        FactionTemplateEntry const* mine = bot->GetFactionTemplateEntry();
        ProfSpot const* best = nullptr;
        float best_d2 = 1500.f * 1500.f;
        for (ProfSpot const& s : it->second)
        {
            if (s.map != bot->GetMapId())
                continue;
            if (mine)
                if (FactionTemplateEntry const* theirs = sFactionTemplateStore.LookupEntry(s.faction))
                    if (!mine->IsFriendlyTo(theirs) && !theirs->IsFriendlyTo(mine))
                        continue;
            float const d2 = bot->GetExactDist2dSq(s.x, s.y);
            if (d2 < best_d2) { best_d2 = d2; best = &s; }
        }
        if (!best)
        {
            std::lock_guard lk(g_prof_mtx);
            g_prof_retry_after[id] = now_ms + 30u * 60u * 1000u;   // none near: try again later, perhaps from elsewhere
            continue;
        }
        {
            std::lock_guard lk(g_prof_mtx);
            g_prof_trip[id] = { true, want->spell, want->skill, best->map, best->x, best->y, best->z, best->entry, now_ms ? now_ms : 1u };
        }
        TC_LOG_INFO("playerbot.v2", "[profession] {} L{} goes to learn {} from trainer {} ({:.0f} y)", bot->GetName(),
            uint32(bot->GetLevel()), want->name, best->entry, std::sqrt(best_d2));
        if (++planned >= 10)
            break;   // a few per pass: the world does not all rush to the trainers at once
    }
}

} // namespace Playerbot::World
