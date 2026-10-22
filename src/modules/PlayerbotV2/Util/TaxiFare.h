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

} // namespace Playerbot::V2::TaxiFare
