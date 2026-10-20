#include "ClassTrainerIndex.h"
#include "Config.h"
#include "DB2Stores.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
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

Index& Built()
{
    Index& idx = TheIndex();
    if (!idx.built)
        Build();
    return idx;
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

} // namespace Playerbot::World
