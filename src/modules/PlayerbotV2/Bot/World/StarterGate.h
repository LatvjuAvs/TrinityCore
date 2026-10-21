// StarterGate - local (WoW Forever local server, M7 "living world"), Playerbot.World.FreshStart: a newcomer below
// Playerbot.World.StarterLeaveLevel (default 4) that stands in its race's starter area (within 350 y of a race start point)
// does not take a quest that leads out of it (every turn-in NPC stands outside: "A Peon's Burden", "Report to Goldshire"). Live 2026-10-21: level 2 bots
// took the breadcrumb, found nothing for their level in Razor Hill and died in loops among level 6-10 mobs.
// Thread-safe (the quest offer scan runs on the snapshot workers); the quest -> turn-in areas table is built once.

#pragma once

#include "Define.h"

class Player;
class Quest;

namespace Playerbot::Local {

// True when the bot should not take this quest yet.
bool StarterQuestTooEarly(Player const* bot, Quest const* quest);

} // namespace Playerbot::Local
