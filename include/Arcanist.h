#ifndef ARCANIST_H
#define ARCANIST_H

#include <memory>
#include <string>

#include "ArcanistAbility.h"
#include "Position.h"
#include "Stats.h"

// AbstractArcanist stores the state and behaviour shared by all player classes.
// The concrete subclasses below supply the starting stats and class name.
class AbstractArcanist {
    std::string className;
    int maxFP;
    int currentFP;
    int basePower;
    int baseWard;
    std::unique_ptr<Stats> stats;
    std::unique_ptr<ArcanistAbility> ability;
    int runeFragments;
    Position position;
    bool cipherGem;
    bool aegisCloak;

  protected:
    AbstractArcanist(
        const std::string &className,
        int maxFP,
        int power,
        int ward,
        const Position &position,
        std::unique_ptr<ArcanistAbility> ability);

  public:
    virtual ~AbstractArcanist() = default;

    // Returns the playable class name shown in the status row.
    std::string getClassName() const;

    // Returns the class's maximum Focus Points.
    int getMaxFP() const;

    // Returns the Arcanist's current Focus Points.
    int getCurrentFP() const;

    // Returns the Arcanist's current attack strength.
    int getPower() const;

    // Returns the Arcanist's current defensive Ward Rating.
    int getWard() const;

    // Returns the total Rune Fragment score collected so far.
    int getRuneFragments() const;

    // Returns the Arcanist's current board position.
    Position getPosition() const;

    // Returns the board symbol for the player.
    char getSymbol() const;

    // Returns the final score after class ability bonuses.
    int getFinalScore() const;

    // Returns whether the Arcanist carries this level's Cipher Gem.
    bool hasCipherGem() const;

    // Returns whether the Arcanist carries the Aegis Cloak.
    bool hasAegisCloak() const;

    // Returns how many times the current class applies one scroll.
    int getScrollEffectMultiplier() const;

    // Lets the class ability transform a scroll before it applies.
    ScrollType transformScroll(ScrollType type, Random &rng) const;

    // Applies class-specific incoming damage changes.
    int modifyIncomingDamage(int damage) const;

    // Moves the Arcanist to an already-validated board position.
    void setPosition(const Position &newPosition);

    // Changes FP while keeping it between 0 and maxFP.
    void changeFP(int amount);

    // Changes Power while preventing negative values.
    void changePower(int amount);

    // Changes Ward Rating while preventing negative values.
    void changeWard(int amount);

    // Adds a temporary +5 Power decorator.
    void addSurgePowerEffect();

    // Adds a temporary +5 Ward decorator.
    void addFortifyWardEffect();

    // Adds a temporary -5 Power decorator.
    void addSapPowerEffect();

    // Adds a temporary -5 Ward decorator.
    void addErodeWardEffect();

    // Removes all temporary stat decorators and restores base Power/Ward.
    void resetTemporaryStats();

    // Adds collected rune value to the score counter.
    void addRuneFragments(int amount);

    // Marks this level's Cipher Gem as carried.
    void collectCipherGem();

    // Marks the Aegis Cloak as carried.
    void collectAegisCloak();

    // Clears level-specific major item state.
    void resetCipherGem();
};

class Sage: public AbstractArcanist {
  public:
    // Creates a Sage with the spec's starting stats.
    explicit Sage(const Position &position);
};

class Hexblade: public AbstractArcanist {
  public:
    // Creates a Hexblade with the spec's starting stats.
    explicit Hexblade(const Position &position);
};

class Warden: public AbstractArcanist {
  public:
    // Creates a Warden with the spec's starting stats.
    explicit Warden(const Position &position);
};

class Voidwalker: public AbstractArcanist {
  public:
    // Creates a Voidwalker with the spec's starting stats.
    explicit Voidwalker(const Position &position);
};

#endif
