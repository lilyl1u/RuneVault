#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Controller.h"
#include "Game.h"

// Reads all rows from a layout file while preserving spaces inside each row.
std::vector<std::string> readLayoutRows(const std::string &path) {
    std::ifstream file{path};
    if (!file) {
        throw std::runtime_error{"Could not open layout file: " + path};
    }

    std::vector<std::string> rows;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        rows.emplace_back(line);
    }

    return rows;
}

// Parses a command-line seed and rejects mixed strings like "12abc".
unsigned int parseSeedArgument(const std::string &text) {
    if (text.empty()) {
        throw std::invalid_argument{"Seed cannot be empty"};
    }

    for (unsigned char ch: text) {
        if (!std::isdigit(ch)) {
            throw std::invalid_argument{"Seed must contain only digits"};
        }
    }

    return static_cast<unsigned int>(std::stoul(text));
}

// Returns true when an argument is shaped like a non-negative integer seed.
bool looksLikeSeed(const std::string &text) {
    if (text.empty()) {
        return false;
    }

    for (unsigned char ch: text) {
        if (!std::isdigit(ch)) {
            return false;
        }
    }

    return true;
}

// Entry point: create the MVC objects and start the controller loop.
int main(int argc, char *argv[]) {
    // ==================== DESIGN PATTERN: MVC ====================
    // main wires together the Model owner (Game) and Controller.
    // TextDisplay is owned by Game for now, so main does not perform rendering itself.
    try {
        if (argc > 3) {
            throw std::invalid_argument{
                "Usage: ./runevault [seed] or ./runevault [layout-file] [seed]"};
        }

        std::unique_ptr<Game> game;

        if (argc == 1) {
            game = std::make_unique<Game>();
        } else if (argc == 2 && looksLikeSeed(argv[1])) {
            game = std::make_unique<Game>(parseSeedArgument(argv[1]));
        } else if (argc == 2) {
            game = std::make_unique<Game>(readLayoutRows(argv[1]));
        } else {
            std::vector<std::string> rows = readLayoutRows(argv[1]);
            game = std::make_unique<Game>(rows, parseSeedArgument(argv[2]));
        }

        Controller controller{*game};
        controller.run(std::cin, std::cout);
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}
