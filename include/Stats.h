#ifndef STATS_H
#define STATS_H

#include <memory>

// ==================== DESIGN PATTERN: Decorator ====================
// Stats is the Component interface in the Decorator pattern.
// BaseStats provides the real values, and decorators wrap it to add temporary effects.
class Stats {
  public:
    virtual ~Stats() = default;

    // Returns the current Power after all wrappers are applied.
    virtual int getPower() const = 0;

    // Returns the current Ward Rating after all wrappers are applied.
    virtual int getWard() const = 0;
};

// Concrete Component: stores the permanent base Power and Ward Rating.
class BaseStats: public Stats {
    int power;
    int ward;

  public:
    // Creates the permanent stat object.
    BaseStats(int power, int ward);

    // Returns permanent Power.
    int getPower() const override;

    // Returns permanent Ward Rating.
    int getWard() const override;
};

// Abstract Decorator: owns another Stats object and forwards unchanged values by default.
class StatsDecorator: public Stats {
  protected:
    std::unique_ptr<Stats> wrapped;

  public:
    // Takes ownership of the Stats object being decorated.
    explicit StatsDecorator(std::unique_ptr<Stats> wrapped);

    // Returns wrapped Power unless a subclass changes it.
    int getPower() const override;

    // Returns wrapped Ward Rating unless a subclass changes it.
    int getWard() const override;
};

// Concrete Decorator: temporary +5 Power from Surge Power.
class SurgePowerStats: public StatsDecorator {
  public:
    // Wraps existing stats with a Power increase.
    explicit SurgePowerStats(std::unique_ptr<Stats> wrapped);

    // Returns Power with +5 applied.
    int getPower() const override;
};

// Concrete Decorator: temporary +5 Ward Rating from Fortify Ward.
class FortifyWardStats: public StatsDecorator {
  public:
    // Wraps existing stats with a Ward increase.
    explicit FortifyWardStats(std::unique_ptr<Stats> wrapped);

    // Returns Ward Rating with +5 applied.
    int getWard() const override;
};

// Concrete Decorator: temporary -5 Power from Sap Power.
class SapPowerStats: public StatsDecorator {
  public:
    // Wraps existing stats with a Power decrease.
    explicit SapPowerStats(std::unique_ptr<Stats> wrapped);

    // Returns Power with -5 applied, never below 0.
    int getPower() const override;
};

// Concrete Decorator: temporary -5 Ward Rating from Erode Ward.
class ErodeWardStats: public StatsDecorator {
  public:
    // Wraps existing stats with a Ward decrease.
    explicit ErodeWardStats(std::unique_ptr<Stats> wrapped);

    // Returns Ward Rating with -5 applied, never below 0.
    int getWard() const override;
};

#endif
