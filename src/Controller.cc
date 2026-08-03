#include "Controller.h"

#include <iostream>
#include <string>

// Stores a non-owning reference to the Game model.
Controller::Controller(Game &game): game{game} {}

// Runs the main input loop: draw, read a command, update Game, redraw.
void Controller::run(std::istream &in, std::ostream &out) {
    game.draw(out);

    std::string command;
    while (!game.shouldQuit() && std::getline(in, command)) {
        if (command.empty()) {
            continue;
        }

        game.handleCommand(command);

        if (!game.shouldQuit()) {
            game.draw(out);
        }
    }
}
