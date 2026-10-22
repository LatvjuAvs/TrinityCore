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
    uint32 trainer_id = 0;
};

// Config switch (default off: upstream behaviour, trainer spells free at a level-up).
bool EarnSpellsEnabled();

// Trainer spells the bot could buy now (level reached, prerequisites known, its class, not known yet). World thread.
uint32 CountLearnableTrainerSpells(Player* bot);

// Nearest class trainer of the bot's class on its map, friendly to it, within max_dist yards; false when none. World thread.
bool NearestClassTrainer(Player* bot, float max_dist, ClassTrainerSpot& out);

// A trip that cannot reach the trainer (an indoor or cave trainer the paths do not reach): the bot buys what it can afford from its
// nearest class trainer as if it had walked there, at the trainer's prices. Queue from any thread; drained on the world thread.
void QueueRemoteTraining(uint64 bot_low);
void DriveRemoteTraining();

// Local (M7): professions learned as a player does. A world bot from level 5 picks a gathering profession that suits its
// class and a second one that goes with it (Herbalism + Alchemy, Mining + Blacksmithing, Skinning + Leatherworking, ...),
// walks to the nearest friendly trainer for the one it lacks and buys Apprentice there (10 copper). Playerbot.Bot.LearnProfessions
// (default 1). The plan is made on the world thread; idle:profession_trip walks it.
struct ProfessionTrip
{
    bool   active = false;
    uint32 spell = 0;            // the Apprentice spell (2366 Herbalism, 2575 Mining, ...)
    uint16 skill = 0;            // its skill line (182, 186, ...)
    uint32 map = 0;
    float  x = 0.f, y = 0.f, z = 0.f;
    uint32 creature_entry = 0;   // the trainer
    uint32 since_ms = 0;
};
bool ProfessionTripFor(uint64 bot_low, ProfessionTrip& out);    // any thread
void EndProfessionTrip(uint64 bot_low, uint32 retry_after_ms);  // any thread; 0 = may plan again at once
void PlanProfessionTrips(uint32 now_ms);                        // world thread, every few seconds

} // namespace Playerbot::World
