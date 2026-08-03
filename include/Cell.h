#ifndef CELL_H
#define CELL_H

// A Cell stores the base terrain of one board square.
// Dynamic objects such as the player, items, and enemies are layered on top by TextDisplay.
enum class CellType {
    Empty,
    WallVertical,
    WallHorizontal,
    Floor,
    Archway,
    Corridor
};

class Cell {
    CellType type;
    int chamberId;

  public:
    Cell(CellType type = CellType::Empty, int chamberId = -1);

    CellType getType() const;
    int getChamberId() const;
    char getSymbol() const;

    // Walkable terrain is anything the player can eventually stand on.
    bool isWalkable() const;

    // Spawning is stricter than walking: objects spawn only on chamber floor tiles.
    bool isSpawnFloor() const;
};

#endif
