#ifndef MOVEMENTSTRATEGY_H
#define MOVEMENTSTRATEGY_H

#include <optional>
#include <string>

#include "Position.h"
#include "Random.h"

class AbstractSpectre;

// MovementContext gives strategies the board facts needed to choose legal movement.
// It is passed by const reference, so strategies do not own or mutate the level directly.
class MovementContext {
  public:
    virtual ~MovementContext() = default;

    // Returns whether a spectre may move to this destination.
    virtual bool canSpectreMoveTo(const AbstractSpectre &spectre, const Position &position) const = 0;

    // BONUS FEATURE: Spectre intelligence
    // Exposes the player location so pathfinding strategies can pursue across the map.
    virtual Position getArcanistPosition() const = 0;

    // BONUS FEATURE: Spectre intelligence
    // Allows pathfinding strategies to reject out-of-bounds positions without knowing the map.
    virtual bool inBounds(const Position &position) const = 0;

    // BONUS FEATURE: Spectre intelligence
    // Returns whether intelligent spectres may plan through this walkable map tile.
    virtual bool canSpectrePathThrough(const AbstractSpectre &spectre, const Position &position) const = 0;
};

// ==================== DESIGN PATTERN: Strategy ====================
// MovementStrategy is the Strategy interface for spectre movement algorithms.
// Each Spectre owns one strategy and delegates movement decisions to it.
class MovementStrategy {
  public:
    virtual ~MovementStrategy() = default;

    // Returns the strategy name for tests/report discussion.
    virtual std::string getName() const = 0;

    // Chooses a destination; nullopt means the spectre stays in place.
    virtual std::optional<Position> chooseMove(
        const AbstractSpectre &spectre,
        const MovementContext &context,
        Random &rng) const = 0;
};

// Concrete Strategy: chooses a random legal adjacent tile in the spawn chamber.
class RandomMovement: public MovementStrategy {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Chooses one random legal adjacent destination, or stays still if none exist.
    std::optional<Position> chooseMove(
        const AbstractSpectre &spectre,
        const MovementContext &context,
        Random &rng) const override;
};

// Concrete Strategy: never moves.
class StationaryMovement: public MovementStrategy {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Always returns nullopt.
    std::optional<Position> chooseMove(
        const AbstractSpectre &spectre,
        const MovementContext &context,
        Random &rng) const override;
};

// BONUS FEATURE: Spectre intelligence - BFS pursuit across rooms and corridors.
class ChaseMovement: public MovementStrategy {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Uses BFS to choose the first step on the shortest path toward the Arcanist.
    std::optional<Position> chooseMove(
        const AbstractSpectre &spectre,
        const MovementContext &context,
        Random &rng) const override;
};

#endif
