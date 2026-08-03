#include "ArcanistAbility.h"

#include <algorithm>

// Default scoring leaves the Rune Fragment total unchanged.
int ArcanistAbility::finalScore(int runeFragments) const {
    return runeFragments;
}

// Default classes apply each scroll once.
int ArcanistAbility::scrollEffectMultiplier() const {
    return 1;
}

// Default classes do not transform scroll type.
ScrollType ArcanistAbility::transformScroll(ScrollType type, Random &rng) const {
    (void) rng;
    return type;
}

// Default classes take the incoming damage unchanged.
int ArcanistAbility::modifyIncomingDamage(int damage) const {
    return damage;
}

// Returns the Sage strategy name.
std::string SageAbility::getName() const {
    return "Sage Ability";
}

// Applies a +50% score bonus using integer division.
int SageAbility::finalScore(int runeFragments) const {
    return runeFragments + runeFragments / 2;
}

// Returns the Hexblade strategy name.
std::string HexbladeAbility::getName() const {
    return "Hexblade Ability";
}

// Hexblade applies each scroll effect twice.
int HexbladeAbility::scrollEffectMultiplier() const {
    return 2;
}

// Returns the Warden strategy name.
std::string WardenAbility::getName() const {
    return "Warden Ability";
}

// Warden reduces incoming damage by 1, while still taking at least 1 damage.
int WardenAbility::modifyIncomingDamage(int damage) const {
    return std::max(1, damage - 1);
}

// Returns the Voidwalker strategy name.
std::string VoidwalkerAbility::getName() const {
    return "Voidwalker Ability";
}

// Voidwalker may flip negative scrolls to the matching positive scroll.
ScrollType VoidwalkerAbility::transformScroll(ScrollType type, Random &rng) const {
    if (!rng.chance(1, 2)) {
        return type;
    }

    if (type == ScrollType::DrainFocus) {
        return ScrollType::AttuneFocus;
    }
    if (type == ScrollType::SapPower) {
        return ScrollType::SurgePower;
    }
    if (type == ScrollType::ErodeWard) {
        return ScrollType::FortifyWard;
    }

    return type;
}
