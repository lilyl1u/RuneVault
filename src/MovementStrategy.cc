#include "MovementStrategy.h"

#include <map>
#include <queue>
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

// Returns the intelligent chase movement strategy name.
std::string ChaseMovement::getName() const {
    return "Chase Movement";
}

// BONUS FEATURE: Spectre intelligence
// Runs BFS over walkable terrain so selected spectres can pursue through corridors.
std::optional<Position> ChaseMovement::chooseMove(
    const AbstractSpectre &spectre,
    const MovementContext &context,
    Random &rng) const {
    (void) rng;

    const std::vector<Direction> directions{
        Direction::North,
        Direction::South,
        Direction::East,
        Direction::West,
        Direction::NorthEast,
        Direction::NorthWest,
        Direction::SouthEast,
        Direction::SouthWest
    };
    Position start = spectre.getPosition();
    Position target = context.getArcanistPosition();

    if (start == target) {
        return std::nullopt;
    }

    std::queue<Position> frontier;
    std::map<std::pair<int, int>, Position> cameFrom;
    std::pair<int, int> startKey{start.row, start.col};

    frontier.emplace(start);
    cameFrom.emplace(startKey, start);

    bool reachedTarget = false;
    while (!frontier.empty() && !reachedTarget) {
        Position current = frontier.front();
        frontier.pop();

        for (Direction direction: directions) {
            Position next = current + directionOffset(direction);
            std::pair<int, int> nextKey{next.row, next.col};

            if (cameFrom.find(nextKey) != cameFrom.end() || !context.inBounds(next)) {
                continue;
            }

            if (next == target) {
                cameFrom.emplace(nextKey, current);
                reachedTarget = true;
                break;
            }

            if (!context.canSpectrePathThrough(spectre, next)) {
                continue;
            }

            frontier.emplace(next);
            cameFrom.emplace(nextKey, current);
        }
    }

    if (!reachedTarget) {
        return std::nullopt;
    }

    Position step = target;
    while (cameFrom.at({step.row, step.col}) != start) {
        step = cameFrom.at({step.row, step.col});
    }

    if (step == target || !context.canSpectrePathThrough(spectre, step)) {
        return std::nullopt;
    }

    return step;
}
