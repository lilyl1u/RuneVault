#include "HostilityStrategy.h"

#include <cstdlib>

#include "Spectre.h"

// Returns whether two positions are within one-block radius.
static bool isAdjacent(const Position &lhs, const Position &rhs) {
    return std::abs(lhs.row - rhs.row) <= 1 && std::abs(lhs.col - rhs.col) <= 1;
}

// Default context response for strategies that do not use guarded-item data.
bool HostilityContext::isArcanistAdjacentToGuardedItem(const Position &anchorPosition) const {
    (void) anchorPosition;
    return false;
}

// Returns the always-hostile strategy name.
std::string AlwaysHostile::getName() const {
    return "Always Hostile";
}

// Attacks only when the Arcanist is in one-block radius.
bool AlwaysHostile::isHostile(
    const AbstractSpectre &spectre,
    const HostilityContext &context) const {
    return isAdjacent(spectre.getPosition(), context.getArcanistPosition());
}

// Returns the neutral strategy name.
std::string NeutralHostility::getName() const {
    return "Neutral";
}

// Neutral spectres do not attack in this chunk.
bool NeutralHostility::isHostile(
    const AbstractSpectre &spectre,
    const HostilityContext &context) const {
    (void) spectre;
    (void) context;
    return false;
}

// Returns the guarded-item strategy name.
std::string GuardedItemHostility::getName() const {
    return "Guarded Item Hostility";
}

// Vault Anchors are hostile only when the Arcanist is adjacent to their guarded item.
bool GuardedItemHostility::isHostile(
    const AbstractSpectre &spectre,
    const HostilityContext &context) const {
    return context.isArcanistAdjacentToGuardedItem(spectre.getPosition());
}
