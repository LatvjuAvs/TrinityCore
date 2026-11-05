#include "RaidEncounters.h"

// Ported from Karrz's raid10-revamp ike3 bots (Karrz1/wow-1.12). Script references (mc/..., bwl/..., ony/...) point at the
// VMaNGOS scripts that Forever's scripts/Custom/Classic instances were ported from; the ids are the same.

namespace Playerbot::Raid {

    // Movement and positioning for Blackwing Lair (map 469). Maintained by the lead engineer.
    void RegisterBlackwingMovement(RaidRules& r)
    {
        // The three drakes (Firemaw 11983, Ebonroc 14601, Flamegor 11981): Wing Buffet and Shadow Flame hit the cone in front of them.
        // The tank holds the front; everyone else stays out of it.
        for (uint32_t drake : { 11983u, 14601u, 11981u })
            r.avoidFrontal.push_back({ drake, 24.0f, 55.0f, Who::NonTank });
        // Firemaw's Flame Buffet 23341 (classic_boss_firemaw.cpp: cast on himself every 1.8-3 s) hits everyone he can see within 100 y for 150
        // fire and adds a stack of +150 fire damage taken, 20 s, refreshed by every hit (70532 spell data, SpellAudit.tsv), so a stack only
        // falls off out of his sight. bwl_firemaw 2026-10-20: nobody left his sight, 36 of 40 died to it 40-47 s in, at 1,400-2,000 a hit.
        // DPS step out at 4-6 stacks and come back clean; healers at 6-8, to a spot that still sees the main tank when there is one; off-tanks
        // wait out of sight, clean for the swap at 8 stacks (TankSwap, RaidEncounterData_BWL.cpp).
        r.breakSight.push_back({ 11983, 23341, 0, Who::Tank });
        r.breakSight.push_back({ 11983, 23341, 6, Who::Healer, /*seeTank*/ true });
        r.breakSight.push_back({ 11983, 23341, 6, Who::Healer });
        r.breakSight.push_back({ 11983, 23341, 4, Who::Melee });
        r.breakSight.push_back({ 11983, 23341, 4, Who::Ranged });
        // Chromaggus: his two breaths are not cones. The client data (70532 SpellEffect: Incinerate 23308, Time Lapse 23310, Corrosive
        // Acid 23313, Ignite Flesh 23315, Frost Burn 23187) targets every enemy around the caster within radius index 12 (100 y), and the
        // area test keeps line of sight, so only a wall saves a raider. bwl_chromaggus 0.1.28c: Frost Burn and Ignite Flesh killed 14
        // non-tanks in one second at 5-33 y, 62 s in, and the raid wiped at 74%. classic_boss_chromaggus.cpp casts breath one 30 s into
        // the fight and breath two at 60 s, each again every 60 s: a breath every 30 s. Everyone but his victim goes out of his sight 5 s
        // before each one is due and stays until 3 s after (the cast is 2 s), and at once whenever he is casting one; then walks back
        // to where it stood. The off-tanks hide too: only his victim and the main tank take the breath (bwl_chromaggus 0.1.28f: an
        // off-tank standing by the main tank died of Ignite Flesh 10 s after him).
        r.hideBeforeBreath.push_back({ 14020, { 23308, 23310, 23313, 23315, 23187 }, 30000, 30000, 5000, 3000, Who::Anyone });
        // The cone below only keeps the raid off his face between breaths.
        r.avoidFrontal.push_back({ 14020, 35.0f, 50.0f, Who::NonTank });
        // Vaelastrasz: his flame breath cleave chains through anyone near the front.
        r.avoidFrontal.push_back({ 13020, 20.0f, 60.0f, Who::NonTank });
        // Broodlord Lashlayer (12017): Blast Wave 23331 is cast on himself (boss_broodlord_lashlayer.cpp:170-173) every 20-35s; it is a
        // point-blank AoE with knockback, 2,625-3,375 fire within 20 yards (vanilla data: radius index 9), so ranged and healers stay out of 22
        // (bwl_broodlord 2026-10-20: four casters died of it standing 10 y off).
        r.avoidEnemy.push_back({ 12017, 22.0f, Who::Ranged });
        r.avoidEnemy.push_back({ 12017, 22.0f, Who::Healer });
        // Nefarian (11583): Shadow Flame 22539 and Cleave 20691 to the front (classic_boss_nefarian.cpp:314,341). Tail Lash 23364 behind
        // him: melee stand at his sides.
        r.avoidFrontal.push_back({ 11583, 25.0f, 55.0f, Who::NonTank });
        r.avoidFrontal.push_back({ 11583, 20.0f, 45.0f, Who::NonTank, 180.0f });
        // Corrupted Infernal (14668, warlock call): Immolation 20153 burns everything close; casters and healers stay off it (ike3).
        r.avoidEnemy.push_back({ 14668, 10.0f, Who::Ranged });
        r.avoidEnemy.push_back({ 14668, 10.0f, Who::Healer });
        // Victor Nefarius' Drakonid packs are handled by the data file (priorities, dispels).
    }
} // namespace Playerbot::Raid
