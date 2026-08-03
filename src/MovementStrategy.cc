#include "MovementStrategy.h"

#include <vector>

#include "Direction.h"
#include "Spectre.h"

// Returns the random movement strategy name.
std::string RandomMovement::getName() const {
    return "Random Movement";
}

// Builds legal adjacent choices and picks one uniformly.
std::optional<Position> RandomMovement::chooseMove(
    const AbstractSpectre &spectre,
    const MovementContext &context,
    Random &rng) const {
    std::vector<Direction> directions{
        Direction::North,
        Direction::South,
        Direction::East,
        Direction::West,
        Direction::NorthEast,
        Direction::NorthWest,
        Direction::SouthEast,
        Direction::SouthWest
    };
    std::vector<Position> choices;

    for (Direction direction: directions) {
        Position candidate = spectre.getPosition() + directionOffset(direction);
        if (context.canSpectreMoveTo(spectre, candidate)) {
            choices.emplace_back(candidate);
        }
    }

    if (choices.empty()) {
        return std::nullopt;
    }

    return choices[rng.range(0, static_cast<int>(choices.size()) - 1)];
}

// Returns the stationary strategy name.
std::string StationaryMovement::getName() const {
    return "Stationary Movement";
}

// Stationary movement always stays put.
std::optional<Position> StationaryMovement::chooseMove(
    const AbstractSpectre &spectre,
    const MovementContext &context,
    Random &rng) const {
    (void) spectre;
    (void) context;
    (void) rng;
    return std::nullopt;
}
