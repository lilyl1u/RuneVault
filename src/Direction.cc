#include "Direction.h"

// Parses one of the eight direction command strings.
bool parseDirection(const std::string &text, Direction &direction) {
    if (text == "no") {
        direction = Direction::North;
    } else if (text == "so") {
        direction = Direction::South;
    } else if (text == "ea") {
        direction = Direction::East;
    } else if (text == "we") {
        direction = Direction::West;
    } else if (text == "ne") {
        direction = Direction::NorthEast;
    } else if (text == "nw") {
        direction = Direction::NorthWest;
    } else if (text == "se") {
        direction = Direction::SouthEast;
    } else if (text == "sw") {
        direction = Direction::SouthWest;
    } else {
        return false;
    }

    return true;
}

// Converts a Direction enum into its row/column offset.
Position directionOffset(Direction direction) {
    switch (direction) {
        case Direction::North:
            return Position{-1, 0};
        case Direction::South:
            return Position{1, 0};
        case Direction::East:
            return Position{0, 1};
        case Direction::West:
            return Position{0, -1};
        case Direction::NorthEast:
            return Position{-1, 1};
        case Direction::NorthWest:
            return Position{-1, -1};
        case Direction::SouthEast:
            return Position{1, 1};
        case Direction::SouthWest:
            return Position{1, -1};
    }

    return Position{0, 0};
}
