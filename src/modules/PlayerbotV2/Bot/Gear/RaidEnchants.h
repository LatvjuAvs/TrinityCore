#pragma once
// Permanent enchants for a raid-ready bot (Playerbot.Gear.RaidEnchants): every equipped piece the role's loadout (RaidLoadout.h) names gets
// its enchant, as a vanilla raider's did. World thread.

#include "Define.h"

class Player;

namespace Playerbot::Gear {

// Enchants the bot's worn items that have no permanent enchant yet. Returns how many it enchanted. Logs "[enchant]" lines: what went on, and
// the names the spell data did not know (once per name).
uint32 ApplyRaidEnchants(Player* p, uint16 spec);

} // namespace Playerbot::Gear
