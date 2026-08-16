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

// EXTENSION: Ritualist
// Default classes opt out of the level-start random scroll effect. Keeping the
// default here means Game can ask every ArcanistAbility the same question.
bool ArcanistAbility::startsLevelWithRandomScrollEffect() const {
    return false;
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

// EXTENSION: Ritualist
// Returns the strategy name for the new fifth class's ability.
std::string RitualistAbility::getName() const {
    return "Ritualist Ability";
}

// EXTENSION: Ritualist
// This is the Strategy hook that turns on the free random scroll at level start.
bool RitualistAbility::startsLevelWithRandomScrollEffect() const {
    return true;
}
