#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>

#include "Cell.h"
#include "Position.h"

// ==================== DESIGN PATTERN: MVC - Model ====================
// Map is part of the Model because it represents core game state.
// The View reads from this class, but Map itself does not print or handle input.
class Map {
    std::vector<std::vector<Cell>> grid;

    void placeRoom(int top, int left, int height, int width, int chamberId);
    void setCell(const Position &pos, CellType type, int chamberId = -1);

  public:
    static const int Height = 25;
    static const int Width = 79;

    Map();

    // ==================== DESIGN PATTERN: Simple Factory-style Helper ====================
    // This named constructor centralizes creation of the default map layout.
    // Later layout-file parsing can be added without spreading map construction logic around.
    static Map createDefault();

    // Creates a map from 25 layout rows; digit objects are treated as floor terrain.
    static Map createFromLayoutRows(const std::vector<std::string> &rows);

    bool inBounds(const Position &pos) const;
    const Cell &getCell(const Position &pos) const;
    std::vector<Position> getSpawnTiles(int chamberId) const;
    std::vector<std::string> toRows() const;
};

#endif
