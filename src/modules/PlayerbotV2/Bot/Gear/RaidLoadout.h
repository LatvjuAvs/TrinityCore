#pragma once
// RaidLoadout - the permanent enchants a vanilla raider wore, by role, as names of the enchanting spells.
//
// Names, not ids: the Forever client data renumbers some vanilla spells, and an enchant that is not in the data is
// then skipped and logged ("[enchant] unresolved ...") instead of putting a wrong enchantment on an item. Each wish
// lists candidate names separated by '|'; the first one the spell data has is used. Pure, no game headers: tested in
// tests/game/ClassicRaidLoadout.cpp.

#include <cstdint>
#include <string_view>
#include <span>

namespace Playerbot::Gear::Loadout {

// Mirrors Gear::RaidRole.
enum class Role : std::uint8_t { Tank, Healer, Caster, Melee, Hunter };

// Equipment slots (EquipmentSlots values).
constexpr std::uint8_t SLOT_HEAD = 0, SLOT_SHOULDERS = 2, SLOT_LEGS = 6, SLOT_CHEST = 4, SLOT_FEET = 7, SLOT_WRISTS = 8, SLOT_HANDS = 9, SLOT_BACK = 14, SLOT_MAINHAND = 15, SLOT_OFFHAND = 16;

struct EnchantWish
{
    std::uint8_t slot;
    std::string_view names;   // candidates, '|' separated, in order of preference
};

// The wishes for a role. `agility_hands` picks the agility gloves (rogue, feral, hunter) over the strength ones, `school` the spell school
// of caster gloves (2 fire, 4 frost, 5 shadow; anything else = no school gloves). Healers take healing power.
std::span<EnchantWish const> WishesFor(Role role, bool agility_hands, std::uint8_t school);

// The first candidate of `names` for which `has(name)` is true; empty when none.
template <typename Has>
std::string_view FirstKnown(std::string_view names, Has has)
{
    while (!names.empty())
    {
        std::size_t const bar = names.find('|');
        std::string_view const one = names.substr(0, bar);
        if (!one.empty() && has(one))
            return one;
        if (bar == std::string_view::npos)
            break;
        names.remove_prefix(bar + 1);
    }
    return {};
}

} // namespace Playerbot::Gear::Loadout
