// QuestParty - local (WoW Forever local server, M7 "living world"): world bots team up to quest, Playerbot.World.QuestParties
// (default off). Every 20 s the matchmaker looks at the world bots that are free (alive, out of combat, no group, no owner, in
// the open world) and pairs those within 100 y in the same zone (a mate out of /say range is invited by whisper), of the same
// faction and nearly the same level, that work on the same quest; a party has 2 or 3 bots and lasts 30 to 60 minutes. The party
// works on one quest at a time (PartyQuest, the leader's quest most members share): the members' quest pickers prefer it, kills
// and quest credit are shared by the core, idle:assist_group_pve makes them help each other, and idle:party_regroup (priority
// 700, above quest pursuit) walks a member more than 70 y away back to its leader. The matchmaker breaks a party up when its time is
// over, its members are apart for a few minutes, their levels drift apart, or a real player joins it (then it is his group).
// World thread for Drive(); IsMember() is safe from the bot workers.

#pragma once

#include "Define.h"
#include "ObjectGuid.h"

namespace Playerbot::V2::QuestParty {

bool Enabled();

// World thread, from Module::OnWorldUpdate: forms and dissolves parties (throttled inside).
void Drive(uint32 now_ms);

// True when the bot is in a party the matchmaker made. Any thread.
bool IsMember(uint64 bot_low);

// The quest the bot's party works on together (0: not in a party, or no quest shared). Any thread.
uint32 PartyQuest(uint64 bot_low);


} // namespace Playerbot::V2::QuestParty
