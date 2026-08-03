#include "Map.h"

#include <cctype>
#include <stdexcept>
#include <vector>

// Creates an empty 25x79 board filled with Empty cells.
Map::Map(): grid(Height, std::vector<Cell>(Width)) {}

// Builds the fixed five-chamber default layout from the specification.
Map Map::createDefault() {
    Map map;

    // The default vault follows the five-room graph from the project spec:
    // 1 connects to 2 and 3, 2 connects to 4, 3 connects to 4, and 4 connects to 5.
    map.placeRoom(2, 4, 5, 11, 1);
    map.placeRoom(2, 23, 5, 11, 2);
    map.placeRoom(9, 4, 5, 11, 3);
    map.placeRoom(9, 23, 5, 11, 4);
    map.placeRoom(16, 23, 5, 11, 5);

    // Chamber 1 <-> Chamber 2.
    map.setCell(Position{4, 14}, CellType::Archway);
    map.setCell(Position{4, 23}, CellType::Archway);
    for (int col = 15; col <= 22; ++col) {
        map.setCell(Position{4, col}, CellType::Corridor);
    }

    // Chamber 1 <-> Chamber 3.
    map.setCell(Position{6, 9}, CellType::Archway);
    map.setCell(Position{9, 9}, CellType::Archway);
    for (int row = 7; row <= 8; ++row) {
        map.setCell(Position{row, 9}, CellType::Corridor);
    }

    // Chamber 2 <-> Chamber 4.
    map.setCell(Position{6, 28}, CellType::Archway);
    map.setCell(Position{9, 28}, CellType::Archway);
    for (int row = 7; row <= 8; ++row) {
        map.setCell(Position{row, 28}, CellType::Corridor);
    }

    // Chamber 3 <-> Chamber 4.
    map.setCell(Position{11, 14}, CellType::Archway);
    map.setCell(Position{11, 23}, CellType::Archway);
    for (int col = 15; col <= 22; ++col) {
        map.setCell(Position{11, col}, CellType::Corridor);
    }

    // Chamber 4 <-> Chamber 5.
    map.setCell(Position{13, 28}, CellType::Archway);
    map.setCell(Position{16, 28}, CellType::Archway);
    for (int row = 14; row <= 15; ++row) {
        map.setCell(Position{row, 28}, CellType::Corridor);
    }

    return map;
}

// Builds a terrain map from 25 text rows supplied by a layout file.
Map Map::createFromLayoutRows(const std::vector<std::string> &rows) {
    if (static_cast<int>(rows.size()) != Height) {
        throw std::invalid_argument{"Layout file must contain exactly 25 rows"};
    }

    Map map;

    for (int row = 0; row < Height; ++row) {
        if (static_cast<int>(rows[row].length()) != Width) {
            throw std::invalid_argument{"Each layout row must contain exactly 79 characters"};
        }

        for (int col = 0; col < Width; ++col) {
            Position pos{row, col};
            char symbol = rows[row][col];

            if (symbol == ' ') {
                map.setCell(pos, CellType::Empty);
            } else if (symbol == '|') {
                map.setCell(pos, CellType::WallVertical);
            } else if (symbol == '-') {
                map.setCell(pos, CellType::WallHorizontal);
            } else if (symbol == '.' || std::isdigit(static_cast<unsigned char>(symbol))) {
                // Digits represent items in the layout, but terrain underneath is floor.
                map.setCell(pos, CellType::Floor);
            } else if (symbol == '+') {
                map.setCell(pos, CellType::Archway);
            } else if (symbol == '#') {
                map.setCell(pos, CellType::Corridor);
            } else {
                throw std::invalid_argument{"Layout contains an unsupported terrain symbol"};
            }
        }
    }

    int chamberId = 1;
    std::vector<std::vector<bool>> visited(Height, std::vector<bool>(Width, false));
    std::vector<Position> neighbours{
        Position{-1, 0},
        Position{1, 0},
        Position{0, -1},
        Position{0, 1}
    };

    for (int row = 0; row < Height; ++row) {
        for (int col = 0; col < Width; ++col) {
            Position start{row, col};
            if (visited[row][col] || map.getCell(start).getType() != CellType::Floor) {
                continue;
            }

            std::vector<Position> pending{start};
            visited[row][col] = true;

            while (!pending.empty()) {
                Position current = pending.back();
                pending.pop_back();
                map.setCell(current, CellType::Floor, chamberId);

                for (const Position &offset: neighbours) {
                    Position next{current.row + offset.row, current.col + offset.col};
                    if (map.inBounds(next) &&
                        !visited[next.row][next.col] &&
                        map.getCell(next).getType() == CellType::Floor) {
                        visited[next.row][next.col] = true;
                        pending.emplace_back(next);
                    }
                }
            }

            ++chamberId;
        }
    }

    return map;
}

// Places one rectangular chamber with walls around the edge and floor inside.
void Map::placeRoom(int top, int left, int height, int width, int chamberId) {
    for (int row = top; row < top + height; ++row) {
        for (int col = left; col < left + width; ++col) {
            Position pos{row, col};

            if (row == top || row == top + height - 1) {
                setCell(pos, CellType::WallHorizontal);
            } else if (col == left || col == left + width - 1) {
                setCell(pos, CellType::WallVertical);
            } else {
                setCell(pos, CellType::Floor, chamberId);
            }
        }
    }
}

// Replaces one terrain cell after checking board bounds.
void Map::setCell(const Position &pos, CellType type, int chamberId) {
    if (!inBounds(pos)) {
        throw std::out_of_range{"Map::setCell position is outside the board"};
    }

    grid[pos.row][pos.col] = Cell{type, chamberId};
}

// Returns whether a position is inside the 25x79 board.
bool Map::inBounds(const Position &pos) const {
    return pos.row >= 0 && pos.row < Height && pos.col >= 0 && pos.col < Width;
}

// Returns a terrain cell, throwing for out-of-bounds positions.
const Cell &Map::getCell(const Position &pos) const {
    if (!inBounds(pos)) {
        throw std::out_of_range{"Map::getCell position is outside the board"};
    }

    return grid[pos.row][pos.col];
}

// Collects every legal spawn tile in one chamber.
std::vector<Position> Map::getSpawnTiles(int chamberId) const {
    std::vector<Position> tiles;

    for (int row = 0; row < Height; ++row) {
        for (int col = 0; col < Width; ++col) {
            Position pos{row, col};
            const Cell &cell = grid[row][col];
            if (cell.isSpawnFloor() && cell.getChamberId() == chamberId) {
                tiles.emplace_back(pos);
            }
        }
    }

    return tiles;
}

// Converts the terrain grid into printable rows.
std::vector<std::string> Map::toRows() const {
    std::vector<std::string> rows;

    for (int row = 0; row < Height; ++row) {
        std::string line;
        for (int col = 0; col < Width; ++col) {
            line += grid[row][col].getSymbol();
        }
        rows.emplace_back(line);
    }

    return rows;
}
