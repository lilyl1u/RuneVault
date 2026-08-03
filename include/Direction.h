#ifndef DIRECTION_H
#define DIRECTION_H

#include <string>

#include "Position.h"

// direction commands 
enum class Direction {
    North,
    South,
    East,
    West,
    NorthEast,
    NorthWest,
    SouthEast,
    SouthWest
};

// parses the command text into corresponding Direction enum
bool parseDirection(const std::string &text, Direction &direction);

// calculates row/column change
Position directionOffset(Direction direction);

#endif
