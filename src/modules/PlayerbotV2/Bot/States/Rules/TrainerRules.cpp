// TrainerRules - Refactor #3 pass 10. Migrates idle:trainer (auto-bulk-
// train at any UNIT_NPC_FLAG_TRAINER within 5y when bot has >= 1g) and
// idle:dual_spec_switch (owner-pinned-role -> off-spec activation) out
// of the State_Idle linear cascade. Both fire at the same priority band
// just below the guild family — utility services for a bot that's
// already parked at a service NPC or has owner-pinned role mismatch.

#include "Bot/IdleRule.h"
#include "Group/GroupSnapshot.h"
#include "Bot/BotAI.h"
#include "Bot/BotSnapshotView.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/BotIntent.h"
#include "Bot/ClassTables.h"
#include "GameTime.h"
#include "Log.h"
#include "UnitDefines.h"
#include "Bot/World/ClassTrainerIndex.h"
#include "Bot/States/MaintainHelpers.h"
#include <cmath>

namespace Playerbot {

namespace {

// ---------- idle:trainer ----------
constexpr int32 kAutoTrainMinGold = 10000;   // 1 gold in copper
constexpr float kTrainerInteract = 5.0f;

bool TrainerGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&,uint32)
{
    // Local (M7): with EarnSpells a low-level bot's spells cost copper, so any money is enough to try.
    if (s.gold() < (::Playerbot::World::EarnSpellsEnabled() ? 1 : kAutoTrainMinGold)) return false;
    auto const* trainer = s.nearest_npc_with_flag(UNIT_NPC_FLAG_TRAINER);
    if (!trainer) return false;
    float bx, by, bz; s.position(bx, by, bz);
    const float dx = trainer->x - bx, dy = trainer->y - by, dz = trainer->z - bz;
    if (dx*dx + dy*dy + dz*dz > kTrainerInteract * kTrainerInteract) return false;
    // Per-NPC-guid 15min backoff after the most recent emit. Critical in
    // capital quarters where the nearest UNIT_NPC_FLAG_TRAINER flips
    // between adjacent NPCs (class trainer / profession expert / secondary
    // supply trainer all within 3-5y of each other). Without per-guid
    // backoff, a bot whose nearest trainer can't actually teach it
    // anything walks back-and-forth between adjacent trainers
    // indefinitely — observed live as bot Uraimus pinging between a
    // Leatherworking expert and an adjacent class trainer after every
    // trainer_buy_all returned ServerRefused.
    const uint32 now_ms = GameTime::GetGameTimeMS();
    if (ai.action_recently_tried(BotAI::ActionKind::TrainerLearn,
                                 trainer->guid.GetCounter(), now_ms))
        return false;
    return true;
}

bool TrainerFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&,BotIntentEmitter& emit, uint32)
{
    auto const* trainer = s.nearest_npc_with_flag(UNIT_NPC_FLAG_TRAINER);
    if (!trainer) return false;
    float bx, by, bz; s.position(bx, by, bz);
    const float dx = trainer->x - bx, dy = trainer->y - by, dz = trainer->z - bz;
    if (dx*dx + dy*dy + dz*dz > kTrainerInteract * kTrainerInteract)
        return false;
    emit.trainer_buy_all(trainer->guid);
    const uint32 now_ms = GameTime::GetGameTimeMS();
    ai.note_action_retry(BotAI::ActionKind::TrainerLearn,
                         trainer->guid.GetCounter(), now_ms);
    ai.set_last_rule_fired("idle:trainer");
    return true;
}

// ---------- idle:class_trainer_trip ----------
// Local (M7, Playerbot.Bot.EarnSpells): walk to the class trainer Module::OnLevelUp picked. Within 5 y the trip ends and
// idle:trainer (priority 230, just below) buys what the bot can afford. Gives up after 20 minutes or on another map.
constexpr uint32 kTripTimeoutMs = 20u * 60u * 1000u;

bool ClassTrainerTripGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    BotAI::TrainerTrip const trip = ai.trainer_trip();
    if (!trip.active) return false;
    if (!s.is_alive() || s.in_combat() || s.is_casting()) return false;
    if (trip.map != s.map_id() || GameTime::GetGameTimeMS() - trip.since_ms > kTripTimeoutMs)
    {
        ai.clear_trainer_trip();
        TC_LOG_INFO("playerbot.v2", "[earn_spells] {} L{} trip to its class trainer ended without arriving ({})", s.name(),
            uint32(s.level()), trip.map != s.map_id() ? "left the map" : "timeout");
        return false;
    }
    return true;
}

bool ClassTrainerTripFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    BotAI::TrainerTrip const trip = ai.trainer_trip();
    float bx, by, bz; s.position(bx, by, bz);
    float const dx = trip.x - bx, dy = trip.y - by;
    float const d2 = dx * dx + dy * dy;
    if (d2 <= kTrainerInteract * kTrainerInteract)
    {
        ai.clear_trainer_trip();   // arrived: idle:trainer takes over
        TC_LOG_INFO("playerbot.v2", "[earn_spells] {} L{} reached its class trainer after {} s", s.name(), uint32(s.level()),
            (GameTime::GetGameTimeMS() - trip.since_ms) / 1000u);
        return false;
    }
    // Close by (the trainer's room): one short direct step into the interact ring; ChunkedWalkToward stops 9 y out.
    // Farther: like the RPG trips (RpgRules.cpp), the travel planner beyond 250 y and a chunked walk below. A single long
    // move_to is refused by the path API (Locked). A failed tick does not end the trip; the 20 minute timeout does.
    uint32 const now_ms = GameTime::GetGameTimeMS();
    float const dist = std::sqrt(d2);
    if (dist <= 30.0f)
    {
        float const scale = (dist - 3.0f) / dist;
        emit.move_to(bx + dx * scale, by + dy * scale, trip.z, /*run*/ true);
        ai.set_last_rule_fired("idle:class_trainer_trip");
        return true;
    }
    if (dist > 250.0f
        && States::DriveTravelPlanTo(s, ai, emit, bx, by, bz, trip.map, trip.x, trip.y, trip.z, /*quest_goal=*/false))
    {
        ai.set_last_rule_fired("idle:class_trainer_trip");
        return true;
    }
    if (States::ChunkedWalkToward(s, ai, emit, trip.x, trip.y, trip.z, "idle:class_trainer_trip", now_ms))
        return true;
    return false;   // nothing this tick; other rules run, the trip stays until it arrives or times out
}

// ---------- idle:dual_spec_switch ----------
bool DualSpecGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&,uint32)
{
    if (!s.is_alive() || s.in_combat() || s.is_casting()) return false;
    if (s.raw().movement.is_mounted) return false;
    const Role pinned = ai.effective_role(s);
    if (pinned == Role::Unknown || pinned == Role::Dps) return false;
    if (pinned == s.my_role()) return false;
    const uint8 cls = s.cls();
    const uint32 target_spec =
        (pinned == Role::Tank)   ? TankSpecForClass(cls) :
        (pinned == Role::Healer) ? HealerSpecForClass(cls) :
                                    0u;
    if (target_spec == 0) return false;
    if (uint32(s.spec()) == target_spec) return false;
    const uint32 ds_now_ms = GameTime::GetGameTimeMS();
    const uint64 ds_key = uint64(s.bot_id());
    return !ai.action_recently_tried(BotAI::ActionKind::DualSpec, ds_key, ds_now_ms);
}

bool DualSpecFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&,BotIntentEmitter& emit, uint32)
{
    const Role pinned = ai.effective_role(s);
    const uint8 cls = s.cls();
    const uint32 target_spec =
        (pinned == Role::Tank)   ? TankSpecForClass(cls) :
        (pinned == Role::Healer) ? HealerSpecForClass(cls) :
                                    0u;
    if (target_spec == 0) return false;
    emit.emit(ActivateSpecIntent{target_spec});
    const uint32 ds_now_ms = GameTime::GetGameTimeMS();
    ai.note_action_retry(BotAI::ActionKind::DualSpec,
                         uint64(s.bot_id()), ds_now_ms);
    ai.set_last_rule_fired("idle:dual_spec_switch");
    return true;
}

} // anonymous namespace

void RegisterTrainerRules(IdleRuleRegistry& r)
{
    {
        IdleRule rule;
        rule.name     = "idle:class_trainer_trip";
        rule.priority = 240;   // local (M7): just above idle:trainer, below quest work
        rule.gate     = &ClassTrainerTripGate;
        rule.fire     = &ClassTrainerTripFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:trainer";
        rule.priority = 230;
        rule.gate     = &TrainerGate;
        rule.fire     = &TrainerFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:dual_spec_switch";
        rule.priority = 228;
        rule.gate     = &DualSpecGate;
        rule.fire     = &DualSpecFire;
        r.register_rule(std::move(rule));
    }
}

} // namespace Playerbot
