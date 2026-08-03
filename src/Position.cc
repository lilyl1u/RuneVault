#include "Position.h"

// Creates a row/column position.
Position::Position(int row, int col): row{row}, col{col} {}

// Compares two positions for exact same row and column.
bool Position::operator==(const Position &other) const {
    return row == other.row && col == other.col;
}

// Compares two positions for different coordinates.
bool Position::operator!=(const Position &other) const {
    return !(*this == other);
}

// Adds two row/column positions, usually position + movement offset.
Position operator+(const Position &lhs, const Position &rhs) {
    return Position{lhs.row + rhs.row, lhs.col + rhs.col};
}
