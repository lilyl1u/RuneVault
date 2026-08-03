#include "TextDisplay.h"

#include <iostream>
#include <string>

namespace {
const std::string Reset = "\033[0m";
const std::string WallColour = "\033[37m";
const std::string FloorColour = "\033[90m";
const std::string CorridorColour = "\033[33m";
const std::string ArcanistColour = "\033[1;36m";
const std::string ScrollColour = "\033[35m";
const std::string FragmentColour = "\033[32m";
const std::string MajorItemColour = "\033[1;33m";
const std::string SpectreColour = "\033[31m";
const std::string StairwayColour = "\033[1;34m";

// Returns the ANSI colour that should be used for a board symbol.
std::string colourForSymbol(char symbol) {
    if (symbol == '@') {
        return ArcanistColour;
    }
    if (symbol == '?') {
        return ScrollColour;
    }
    if (symbol == '*') {
        return FragmentColour;
    }
    if (symbol == 'C' || symbol == 'G') {
        return MajorItemColour;
    }
    if (symbol == '\\') {
        return StairwayColour;
    }
    if (symbol == '+' || symbol == '#') {
        return CorridorColour;
    }
    if (symbol == '|' || symbol == '-') {
        return WallColour;
    }
    if (symbol == '.') {
        return FloorColour;
    }
    if (symbol == 'W' || symbol == 'B' || symbol == 'R' ||
        symbol == 'S' || symbol == 'L' || symbol == 'A' ||
        symbol == 'X') {
        return SpectreColour;
    }

    return "";
}

// Prints one symbol with ANSI colour, keeping spaces uncoloured for readability.
void printColouredSymbol(std::ostream &out, char symbol) {
    std::string colour = colourForSymbol(symbol);
    if (colour.empty()) {
        out << symbol;
    } else {
        out << colour << symbol << Reset;
    }
}
}

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
        for (char symbol: row) {
            printColouredSymbol(out, symbol);
        }
        out << '\n';
    }

    // The spec reserves the bottom 5 rows for status information.
    // More detailed player stats will be added once the Arcanist exists.
    out << status << '\n';
    out << "Commands: no so ea we ne nw se sw | u dir use | a dir attack | r restart | q quit" << '\n';
    out << "Message: " << message << '\n';
    out << '\n';
    out << "Command: " << '\n';
}
