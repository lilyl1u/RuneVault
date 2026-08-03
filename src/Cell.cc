#include "Cell.h"

// Creates a terrain cell with an optional chamber id.
Cell::Cell(CellType type, int chamberId): type{type}, chamberId{chamberId} {}

// Returns the terrain type stored in this cell.
CellType Cell::getType() const {
    return type;
}

// Returns the chamber id, or -1 for non-chamber cells.
int Cell::getChamberId() const {
    return chamberId;
}

// Converts terrain type into the display character.
char Cell::getSymbol() const {
    switch (type) {
        case CellType::Empty:
            return ' ';
        case CellType::WallVertical:
            return '|';
        case CellType::WallHorizontal:
            return '-';
        case CellType::Floor:
            return '.';
        case CellType::Archway:
            return '+';
        case CellType::Corridor:
            return '#';
    }
    return ' ';
}

// Reports whether the player can stand on this terrain.
bool Cell::isWalkable() const {
    return type == CellType::Floor ||
           type == CellType::Archway ||
           type == CellType::Corridor;
}

// Reports whether items/enemies may spawn on this cell.
bool Cell::isSpawnFloor() const {
    return type == CellType::Floor && chamberId >= 1;
}
