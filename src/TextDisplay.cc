#include "TextDisplay.h"

#include <iostream>

// Draws terrain, item objects, the Arcanist overlay, and the 5 status rows.
void TextDisplay::draw(
    std::ostream &out,
    const VaultLevel &level,
    const std::string &status,
    const std::string &message,
    const std::optional<Position> &arcanistPosition,
    const std::vector<std::pair<Position, char>> &items,
    const std::vector<std::pair<Position, char>> &spectres) const {
    std::vector<std::string> rows = level.getMap().toRows();

    if (level.isStairwayVisible() && level.getMap().inBounds(level.getHiddenStairway())) {
        Position stairway = level.getHiddenStairway();
        rows[stairway.row][stairway.col] = '\\';
    }

    for (const auto &object: items) {
        if (level.getMap().inBounds(object.first)) {
            rows[object.first.row][object.first.col] = object.second;
        }
    }

    for (const auto &object: spectres) {
        if (level.getMap().inBounds(object.first)) {
            rows[object.first.row][object.first.col] = object.second;
        }
    }

    // The View layers dynamic symbols, like spectres and the Arcanist, over terrain rows.
    // The Model still owns the actual position data.
    if (arcanistPosition && level.getMap().inBounds(arcanistPosition.value())) {
        const Position pos = arcanistPosition.value();
        rows[pos.row][pos.col] = '@';
    }

    for (const std::string &row: rows) {
        out << row << '\n';
    }

    // The spec reserves the bottom 5 rows for status information.
    // More detailed player stats will be added once the Arcanist exists.
    out << status << '\n';
    out << "Commands: no so ea we ne nw se sw | u dir use | a dir attack | r restart | q quit" << '\n';
    out << "Message: " << message << '\n';
    out << '\n';
    out << "Command: " << '\n';
}
