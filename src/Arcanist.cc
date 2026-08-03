#include "Arcanist.h"

#include <algorithm>

// Stores shared Arcanist data and initializes current FP to the class maximum.
AbstractArcanist::AbstractArcanist(
    const std::string &className,
    int maxFP,
    int power,
    int ward,
    const Position &position,
    std::unique_ptr<ArcanistAbility> ability):
    className{className},
    maxFP{maxFP},
    currentFP{maxFP},
    basePower{power},
    baseWard{ward},
    stats{std::make_unique<BaseStats>(power, ward)},
    ability{std::move(ability)},
    runeFragments{0},
    position{position},
    cipherGem{false},
    aegisCloak{false} {}

// Returns the class name used in status output.
std::string AbstractArcanist::getClassName() const {
    return className;
}

// Returns the Arcanist's maximum FP.
int AbstractArcanist::getMaxFP() const {
    return maxFP;
}

// Returns the Arcanist's current FP.
int AbstractArcanist::getCurrentFP() const {
    return currentFP;
}

// Returns the Arcanist's current Power.
int AbstractArcanist::getPower() const {
    return stats->getPower();
}

// Returns the Arcanist's current Ward Rating.
int AbstractArcanist::getWard() const {
    return stats->getWard();
}

// Returns the collected Rune Fragment total.
int AbstractArcanist::getRuneFragments() const {
    return runeFragments;
}

// Returns the Arcanist's current board position.
Position AbstractArcanist::getPosition() const {
    return position;
}

// Returns the player display symbol.
char AbstractArcanist::getSymbol() const {
    return '@';
}

// Delegates scoring to the class ability Strategy.
int AbstractArcanist::getFinalScore() const {
    // ==================== DESIGN PATTERN: Strategy ====================
    // AbstractArcanist does not check whether it is a Sage; it asks its ability.
    return ability->finalScore(runeFragments);
}

// Returns whether this level's Cipher Gem is carried.
bool AbstractArcanist::hasCipherGem() const {
    return cipherGem;
}

// Returns whether the Aegis Cloak is carried.
bool AbstractArcanist::hasAegisCloak() const {
    return aegisCloak;
}

// Delegates scroll multiplier behaviour to the class ability Strategy.
int AbstractArcanist::getScrollEffectMultiplier() const {
    // ==================== DESIGN PATTERN: Strategy ====================
    // Hexblade-specific logic lives in HexbladeAbility, not in Game.
    return ability->scrollEffectMultiplier();
}

// Delegates scroll transformation behaviour to the class ability Strategy.
ScrollType AbstractArcanist::transformScroll(ScrollType type, Random &rng) const {
    // ==================== DESIGN PATTERN: Strategy ====================
    // Voidwalker-specific scroll conversion lives in VoidwalkerAbility.
    return ability->transformScroll(type, rng);
}

// Delegates incoming damage modification to the class ability Strategy.
int AbstractArcanist::modifyIncomingDamage(int damage) const {
    // ==================== DESIGN PATTERN: Strategy ====================
    // Warden damage reduction lives in WardenAbility.
    return ability->modifyIncomingDamage(damage);
}

// Updates the Arcanist's position after Game validates the move.
void AbstractArcanist::setPosition(const Position &newPosition) {
    position = newPosition;
}

// Changes FP while enforcing the [0, maxFP] range.
void AbstractArcanist::changeFP(int amount) {
    currentFP = std::max(0, std::min(maxFP, currentFP + amount));
}

// Changes Power while preventing negative values.
void AbstractArcanist::changePower(int amount) {
    basePower = std::max(0, basePower + amount);
    resetTemporaryStats();
}

// Changes Ward Rating while preventing negative values.
void AbstractArcanist::changeWard(int amount) {
    baseWard = std::max(0, baseWard + amount);
    resetTemporaryStats();
}

// Adds a temporary +5 Power decorator to the current stat chain.
void AbstractArcanist::addSurgePowerEffect() {
    // ==================== DESIGN PATTERN: Decorator ====================
    // The new decorator owns the previous Stats object, forming a runtime stack.
    stats = std::make_unique<SurgePowerStats>(std::move(stats));
}

// Adds a temporary +5 Ward Rating decorator to the current stat chain.
void AbstractArcanist::addFortifyWardEffect() {
    // ==================== DESIGN PATTERN: Decorator ====================
    // Each scroll adds one wrapper without changing the base stats.
    stats = std::make_unique<FortifyWardStats>(std::move(stats));
}

// Adds a temporary -5 Power decorator to the current stat chain.
void AbstractArcanist::addSapPowerEffect() {
    // ==================== DESIGN PATTERN: Decorator ====================
    // Negative effects are also decorators, so they stack with positive effects.
    stats = std::make_unique<SapPowerStats>(std::move(stats));
}

// Adds a temporary -5 Ward Rating decorator to the current stat chain.
void AbstractArcanist::addErodeWardEffect() {
    // ==================== DESIGN PATTERN: Decorator ====================
    // The decorator itself handles the floor at 0.
    stats = std::make_unique<ErodeWardStats>(std::move(stats));
}

// Clears all temporary decorators by rebuilding from the permanent base stats.
void AbstractArcanist::resetTemporaryStats() {
    stats = std::make_unique<BaseStats>(basePower, baseWard);
}

// Adds to the collected Rune Fragment total.
void AbstractArcanist::addRuneFragments(int amount) {
    runeFragments = std::max(0, runeFragments + amount);
}

// Marks the Cipher Gem as carried.
void AbstractArcanist::collectCipherGem() {
    cipherGem = true;
}

// Marks the Aegis Cloak as carried.
void AbstractArcanist::collectAegisCloak() {
    aegisCloak = true;
}

// Clears level-specific Cipher Gem ownership.
void AbstractArcanist::resetCipherGem() {
    cipherGem = false;
}

// Creates a Sage with the required starting stats.
Sage::Sage(const Position &position):
    AbstractArcanist{"Sage", 120, 18, 18, position, std::make_unique<SageAbility>()} {}

// Creates a Hexblade with the required starting stats.
Hexblade::Hexblade(const Position &position):
    AbstractArcanist{"Hexblade", 100, 24, 14, position, std::make_unique<HexbladeAbility>()} {}

// Creates a Warden with the required starting stats.
Warden::Warden(const Position &position):
    AbstractArcanist{"Warden", 150, 16, 22, position, std::make_unique<WardenAbility>()} {}

// Creates a Voidwalker with the required starting stats.
Voidwalker::Voidwalker(const Position &position):
    AbstractArcanist{"Voidwalker", 110, 22, 12, position, std::make_unique<VoidwalkerAbility>()} {}
