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

// Local (M7): the mirror of upstream's grey-quest skip. A quest more than Playerbot.Bot.SoloQuestLevelMargin (default 2;
// -1 = off) levels above a solo bot, or that margin + 2 in a group, is red for it: live 2026-10-21 level 3-4 bots took
// 784 "Vanquish the Betrayers" in Razor Hill and died in loops among the level 8-10 Kul Tiras Sailors.
bool QuestTooHardForBot(Player const* bot, Quest const* quest);

// Local (M7), Playerbot.World.SkipFleetStuckQuests (default 1): quests the bot fleet takes but almost never finishes (30+ bot
// holders, under 5% of holders ever turned it in; live 2026-10-21: "The Fargodeep Mine" 181 holders, 0 turned in; "Lazy
// Peons" 173, 1). BotAI::quest_blacklisted reports them, so the offer scan and the picker skip them, as a player drops a
// quest they cannot do. Refreshed from the characters database every 10 minutes on the world thread; read on any thread.
bool FleetStuckQuest(uint32 quest_id);
void RefreshFleetStuckQuests(uint32 now_ms);   // world thread

} // namespace Playerbot::Local
