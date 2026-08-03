#ifndef ARCANISTABILITY_H
#define ARCANISTABILITY_H

#include <string>

#include "Item.h"
#include "Random.h"

// ==================== DESIGN PATTERN: Strategy ====================
// ArcanistAbility is the Strategy interface for class-specific behaviour.
// AbstractArcanist owns one ability object and delegates class rules to it.
class ArcanistAbility {
  public:
    virtual ~ArcanistAbility() = default;

    // Returns a short ability name for debugging/report discussion.
    virtual std::string getName() const = 0;

    // Adjusts final score; default classes return the fragment total unchanged.
    virtual int finalScore(int runeFragments) const;

    // Returns how many times one scroll effect should be applied.
    virtual int scrollEffectMultiplier() const;

    // Allows a class to transform a scroll before use.
    virtual ScrollType transformScroll(ScrollType type, Random &rng) const;

    // Allows a class to reduce incoming damage before item effects.
    virtual int modifyIncomingDamage(int damage) const;
};

// Concrete Strategy: Sage gets +50% final score.
class SageAbility: public ArcanistAbility {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Applies the Sage scoring bonus.
    int finalScore(int runeFragments) const override;
};

// Concrete Strategy: Hexblade doubles scroll effects.
class HexbladeAbility: public ArcanistAbility {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Applies each scroll effect twice.
    int scrollEffectMultiplier() const override;
};

// Concrete Strategy: Warden reduces incoming damage by 1.
class WardenAbility: public ArcanistAbility {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Reduces damage by 1, with a minimum of 1.
    int modifyIncomingDamage(int damage) const override;
};

// Concrete Strategy: Voidwalker can convert negative scrolls to positive scrolls.
class VoidwalkerAbility: public ArcanistAbility {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Converts negative scrolls to their positive counterpart 50% of the time.
    ScrollType transformScroll(ScrollType type, Random &rng) const override;
};

#endif
