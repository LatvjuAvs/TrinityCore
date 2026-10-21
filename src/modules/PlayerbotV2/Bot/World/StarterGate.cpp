#include "StarterGate.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Player.h"
#include "QuestDef.h"
#include <algorithm>
#include <cstdlib>
#include <string>
#include <array>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace Playerbot::Local {

namespace {

// Race start points (playercreateinfo; races 1-8 share six) and the radius that holds each starter area's quest NPCs but not
// the next town's (Goldshire is ~550 y from Northshire's point, Razor Hill ~900 y from the Valley of Trials').
struct StartPoint { uint32 map; float x, y; };
constexpr std::array<StartPoint, 6> kStarts = {{
    { 0, -8950.f, -132.f },   // Northshire
    { 1, -619.f, -4252.f },   // Valley of Trials
    { 0, -6240.f, 331.f },    // Coldridge Valley
    { 1, 10311.f, 831.f },    // Shadowglen
    { 0, 1676.f, 1677.f },    // Deathknell
    { 1, -2918.f, -258.f },   // Camp Narache
}};
constexpr float kStarterRadius = 350.0f;

struct Spot { uint32 map; float x, y; };
std::unordered_map<uint32, std::vector<Spot>> g_enders;   // quest -> where its turn-in NPCs and objects stand
std::once_flag g_once;

void Build()
{
    if (QueryResult r = WorldDatabase.Query(
            "SELECT e.quest, c.map, c.position_x, c.position_y FROM creature_questender e JOIN creature c ON c.id = e.id "
            "UNION ALL SELECT e.quest, g.map, g.position_x, g.position_y FROM gameobject_questender e JOIN gameobject g ON g.id = e.id"))
    {
        do
        {
            Field* f = r->Fetch();
            g_enders[f[0].GetUInt32()].push_back({ f[1].GetUInt16(), f[2].GetFloat(), f[3].GetFloat() });
        } while (r->NextRow());
    }
    TC_LOG_INFO("playerbot.v2", "[starter_gate] turn-in places known for {} quests", g_enders.size());
}

bool Inside(StartPoint const& sp, uint32 map, float x, float y)
{
    float const dx = x - sp.x, dy = y - sp.y;
    return map == sp.map && dx * dx + dy * dy <= kStarterRadius * kStarterRadius;
}

} // namespace

bool StarterQuestTooEarly(Player const* bot, Quest const* quest)
{
    static bool const fresh = sConfigMgr->GetBoolDefault("Playerbot.World.FreshStart", false, true);
    static uint32 const leave_level = uint32(std::max(0, sConfigMgr->GetIntDefault("Playerbot.World.StarterLeaveLevel", 4, true)));
    if (!fresh || !bot || !quest || bot->GetLevel() >= leave_level)
        return false;
    StartPoint const* home = nullptr;
    for (StartPoint const& sp : kStarts)
        if (Inside(sp, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY()))
            home = &sp;
    if (!home)
        return false;
    std::call_once(g_once, Build);
    auto it = g_enders.find(quest->GetQuestId());
    if (it == g_enders.end() || it->second.empty())
        return false;
    // Leads out only when no turn-in place lies inside the bot's starter area.
    return std::none_of(it->second.begin(), it->second.end(), [home](Spot const& sp) { return Inside(*home, sp.map, sp.x, sp.y); });
}

bool QuestTooHardForBot(Player const* bot, Quest const* quest)
{
    static int32 const margin = sConfigMgr->GetIntDefault("Playerbot.Bot.SoloQuestLevelMargin", 2, true);
    if (margin < 0 || !bot || !quest || quest->GetZoneOrSort() < 0)   // class and profession quests keep their own rules
        return false;
    int32 const ql = bot->GetQuestLevel(quest);
    return ql > 0 && ql > int32(bot->GetLevel()) + margin + (bot->GetGroup() ? 2 : 0);
}

namespace {
std::shared_mutex g_stuck_mtx;
std::unordered_set<uint32> g_stuck;
uint32 g_stuck_refreshed_ms = 0;
}

bool FleetStuckQuest(uint32 quest_id)
{
    std::shared_lock<std::shared_mutex> lk(g_stuck_mtx);
    return g_stuck.contains(quest_id);
}

void RefreshFleetStuckQuests(uint32 now_ms)
{
    static bool const enabled = sConfigMgr->GetBoolDefault("Playerbot.World.SkipFleetStuckQuests", true, true);
    if (!enabled || (g_stuck_refreshed_ms && now_ms - g_stuck_refreshed_ms < 10u * 60u * 1000u))
        return;
    g_stuck_refreshed_ms = now_ms ? now_ms : 1u;
    QueryResult r = CharacterDatabase.Query(
        "SELECT q.quest, COUNT(*) AS holders, "
        "(SELECT COUNT(*) FROM character_queststatus_rewarded rw JOIN playerbot_v2_character pr ON pr.character_guid_low = rw.guid "
        " WHERE rw.quest = q.quest) AS rewarded "
        "FROM character_queststatus q JOIN playerbot_v2_character pv ON pv.character_guid_low = q.guid "
        "GROUP BY q.quest HAVING holders >= 30 AND rewarded * 20 < holders");
    // Area-trigger (explore) quests are left out since bots report trigger entries (2026-10-22); before that no bot had ever
    // turned one in, so their statistics say nothing yet.
    static std::unordered_set<uint32> const s_trigger_quests = []
    {
        std::unordered_set<uint32> out;
        if (QueryResult at = WorldDatabase.Query("SELECT DISTINCT quest FROM areatrigger_involvedrelation"))
            do out.insert(at->Fetch()[0].GetUInt32()); while (at->NextRow());
        return out;
    }();
    // Playerbot.World.FleetStuckRetry: quest ids (comma separated) kept off the list, to test a fix on them.
    static std::unordered_set<uint32> const s_retry = []
    {
        std::unordered_set<uint32> out;
        std::string const v = sConfigMgr->GetStringDefault("Playerbot.World.FleetStuckRetry", "", true);
        size_t i = 0;
        while (i < v.size())
        {
            size_t j = v.find(',', i);
            if (j == std::string::npos) j = v.size();
            if (uint32 const q = uint32(std::strtoul(v.substr(i, j - i).c_str(), nullptr, 10))) out.insert(q);
            i = j + 1;
        }
        return out;
    }();
    std::unordered_set<uint32> fresh;
    std::string list;
    if (r)
        do
        {
            Field* f = r->Fetch();
            if (s_trigger_quests.contains(f[0].GetUInt32()) || s_retry.contains(f[0].GetUInt32()))
                continue;
            fresh.insert(f[0].GetUInt32());
            if (list.size() < 400)
                list += fmt::format("{}({}/{}) ", f[0].GetUInt32(), f[2].GetUInt64(), f[1].GetUInt64());
        } while (r->NextRow());
    size_t const n = fresh.size();
    {
        std::unique_lock<std::shared_mutex> lk(g_stuck_mtx);
        g_stuck.swap(fresh);
    }
    TC_LOG_INFO("playerbot.v2", "[fleet_stuck_quests] {} quests skipped by bots (quest(turned in/holders)): {}", n, list);
}

} // namespace Playerbot::Local
