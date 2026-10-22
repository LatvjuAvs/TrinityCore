// TaxiFare - local (WoW Forever local server, M7 "living world"): under Playerbot.World.FreshStart a bot pays its own way.
// Upstream topped a bot up to 5 gold before every flight and covered any repair it could not afford, so level 3 bots that
// took one flight carried 4 g 99 s (2026-10-22: 17 bots). In a fresh world those stipends are off: the router and the
// proactive taxi pick only flights the bot can pay for, otherwise it walks, and a poor bot repairs what it can afford.

#pragma once

#include "Config.h"
#include "ObjectMgr.h"
#include "Player.h"
#include <vector>

namespace Playerbot::V2::TaxiFare {

inline bool PayOwnWay()
{
    static bool const fresh = sConfigMgr->GetBoolDefault("Playerbot.World.FreshStart", false, true);
    return fresh;
}

// The fare of a flight along `nodes` (TaxiNodes ids, start first), as Player::ActivateTaxiPathTo charges it before the
// reputation discount (so never less than the real fare). 0 when a hop has no path.
inline uint64 RouteFare(Player const* p, std::vector<uint32> const& nodes)
{
    uint64 total = 0;
    for (size_t i = 1; i < nodes.size(); ++i)
    {
        uint32 path = 0, cost = 0;
        sObjectMgr->GetTaxiPath(nodes[i - 1], nodes[i], path, cost);
        if (!path)
            return 0;
        total += cost;
    }
    return p ? p->ApplyClassicLegacyPriceMod(total, 4) : total;
}

// Too poor to repair even a quarter of its gear: a player in that state keeps playing on broken gear and repairs once he has
// earned the money. The critical repair walk and the flee to repair stand down for it (2026-10-22: Gemis, level 2 priest,
// 10 copper, all gear at 0%, stood at idle:critical_repair_approach and gave up on every mob; 1,370 critical repair
// lines and 488 flees to repair in 49 minutes after the free repair was switched off).
inline bool TooPoorToRepair(int64 money, uint32 repair_cost)
{
    return PayOwnWay() && repair_cost > 0 && money * 4 < int64(repair_cost);
}

} // namespace Playerbot::V2::TaxiFare
