#include "StarterGate.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Player.h"
#include "QuestDef.h"
#include <algorithm>
#include <array>
#include <mutex>
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

} // namespace Playerbot::Local
