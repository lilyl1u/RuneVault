#ifndef POSITION_H
#define POSITION_H

struct Position {

    int row;
    int col;

    Position(int row = 0, int col = 0);

    // checking if 2 game objects occupy the same cell
    bool operator==(const Position &other) const;
    bool operator!=(const Position &other) const;
};

// movement offset for a position
Position operator+(const Position &lhs, const Position &rhs);

#endif
