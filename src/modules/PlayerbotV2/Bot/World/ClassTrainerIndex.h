// ClassTrainerIndex - local (WoW Forever local server, M7 "living world"): where a bot's class trainers stand and what they
// teach, for Playerbot.Bot.EarnSpells. With it on a bot no longer gets its trainer spells for free at a level-up: it walks to
// the nearest friendly trainer of its class (idle:class_trainer_trip) and buys them there (idle:trainer), paying like a
// player. Built once from the world database on the world thread.

#pragma once

#include "Define.h"
#include <vector>

class Player;

namespace Playerbot::World {

struct ClassTrainerSpot
{
    uint32 map = 0;
    float  x = 0.f, y = 0.f, z = 0.f;
    uint32 creature_entry = 0;
};

// Config switch (default off: upstream behaviour, trainer spells free at a level-up).
bool EarnSpellsEnabled();

// Trainer spells the bot could buy now (level reached, prerequisites known, its class, not known yet). World thread.
uint32 CountLearnableTrainerSpells(Player* bot);

// Nearest class trainer of the bot's class on its map, friendly to it, within max_dist yards; false when none. World thread.
bool NearestClassTrainer(Player* bot, float max_dist, ClassTrainerSpot& out);

} // namespace Playerbot::World
