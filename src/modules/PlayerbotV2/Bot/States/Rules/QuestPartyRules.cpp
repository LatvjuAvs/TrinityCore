// QuestPartyRules - local (WoW Forever local server, M7 "living world"): idle:party_regroup keeps a quest party
// (Fleet/QuestParty.h, Playerbot.World.QuestParties) together. A member more than 45 y from its leader, out of combat, walks back
// until it is within 12 y; otherwise every member quests with its own rules, which for bots on the same quest means the same
// mob camp. Registered from RegisterTrainerRules so IdleRule.h (included everywhere) stays unchanged.

#include "Bot/IdleRule.h"
#include "Group/GroupSnapshot.h"
#include "Bot/BotAI.h"
#include "Bot/BotSnapshotView.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/States/MaintainHelpers.h"
#include "Fleet/QuestParty.h"
#include "GameTime.h"
#include <cmath>
#include <cstring>

namespace Playerbot {

namespace {

constexpr float kRegroupFar  = 45.0f;
constexpr float kRegroupNear = 12.0f;

GroupMemberSummary const* PartyLeader(BotSnapshotView const& s, GroupSnapshotView const& g)
{
    if (!g.exists() || g.leader() == s.raw().guid) return nullptr;
    if (auto const* ms = g.members())
        for (auto const& m : *ms)
            if (m.guid == g.leader())
                return &m;
    return nullptr;
}

bool PartyRegroupGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const& g, uint32)
{
    if (!s.is_alive() || s.in_combat() || s.is_casting() || s.is_in_instance() || s.in_battleground()) return false;
    if (!g.exists() || !V2::QuestParty::IsMember(s.bot_id())) return false;
    GroupMemberSummary const* lead = PartyLeader(s, g);
    if (!lead || lead->map_id != s.map_id() || lead->hp <= 0) return false;
    float bx, by, bz; s.position(bx, by, bz);
    float const d = std::hypot(lead->x - bx, lead->y - by);
    bool const regrouping = ai.last_rule_fired() && std::strcmp(ai.last_rule_fired(), "idle:party_regroup") == 0;
    return d > kRegroupFar || (regrouping && d > kRegroupNear);
}

bool PartyRegroupFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const& g, BotIntentEmitter& emit, uint32)
{
    GroupMemberSummary const* lead = PartyLeader(s, g);
    if (!lead) return false;
    float bx, by, bz; s.position(bx, by, bz);
    float const d = std::hypot(lead->x - bx, lead->y - by);
    if (d <= 60.0f)
    {
        if (emit.move_to(lead->x, lead->y, lead->z, /*run*/ true) || emit.move_already_emitted(lead->x, lead->y, lead->z))
        {
            ai.set_last_rule_fired("idle:party_regroup");
            return true;
        }
    }
    else if (States::DriveTravelPlanTo(s, ai, emit, bx, by, bz, lead->map_id, lead->x, lead->y, lead->z, /*quest_goal=*/false))
    {
        ai.set_last_rule_fired("idle:party_regroup");
        return true;
    }
    return States::ChunkedWalkToward(s, ai, emit, lead->x, lead->y, lead->z, "idle:party_regroup", GameTime::GetGameTimeMS());
}

} // namespace

void RegisterQuestPartyRules(IdleRuleRegistry& r)
{
    IdleRule rule;
    rule.name     = "idle:party_regroup";
    rule.priority = 235;   // below the class trainer trip (240), above idle:trainer (230) and the quest work in autoact (50)
    rule.gate     = &PartyRegroupGate;
    rule.fire     = &PartyRegroupFire;
    r.register_rule(std::move(rule));
}

} // namespace Playerbot
