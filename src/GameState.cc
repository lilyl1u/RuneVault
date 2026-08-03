#include "GameState.h"

#include "Direction.h"
#include "Game.h"

#include <sstream>
#include <string>

// Default enter hook does nothing for states that do not need setup.
void GameState::onEnter(Game &game) {
    (void) game;
}

// Default exit hook does nothing for states that do not need cleanup.
void GameState::onExit(Game &game) {
    (void) game;
}

// Names the initial class-selection phase.
std::string ClassSelectionState::getName() const {
    return "Class Selection";
}

// Shows the class-selection prompt when this state becomes active.
void ClassSelectionState::onEnter(Game &game) {
    game.setMessage("Choose class: s=Sage, h=Hexblade, w=Warden, v=Voidwalker. q quits.");
}

// Handles only commands that make sense before an Arcanist exists.
void ClassSelectionState::handleCommand(Game &game, const std::string &command) {
    if (command == "s" || command == "h" || command == "w" || command == "v") {
        game.selectClass(command[0]);
    } else if (command == "q") {
        game.quit();
    } else if (command == "r") {
        game.restart();
    } else {
        game.setMessage("Invalid class. Enter s, h, w, v, r, or q.");
    }
}

// Names the active gameplay phase.
std::string PlayingState::getName() const {
    return "Playing";
}

// Shows the gameplay-start message when entering play.
void PlayingState::onEnter(Game &game) {
    game.setMessage("Class selected. Find the Cipher Gem and descend.");
}

// Parses gameplay commands and delegates actual model changes to Game.
void PlayingState::handleCommand(Game &game, const std::string &command) {
    Direction direction = Direction::North;
    std::istringstream input{command};
    std::string action;
    std::string argument;
    std::string extra;

    if (command == "q") {
        game.quit();
    } else if (command == "r") {
        game.restart();
    } else if (parseDirection(command, direction)) {
        // ==================== DESIGN PATTERN: State ====================
        // PlayingState decides movement commands are valid in the playing phase.
        // The actual movement rule stays in Game, which is the Model.
        game.moveArcanist(direction);
    } else if (input >> action) {
        if (action == "u") {
            if (!(input >> argument) || (input >> extra) || !parseDirection(argument, direction)) {
                game.setMessage("Use scroll with: u direction.");
            } else {
                game.useScroll(direction);
            }
        } else if (action == "a") {
            if (!(input >> argument) || (input >> extra) || !parseDirection(argument, direction)) {
                game.setMessage("Attack with: a direction.");
            } else {
                game.attackSpectre(direction);
            }
        } else {
            game.setMessage("Unknown gameplay command. Use movement, r to restart, or q to quit.");
        }
    } else {
        game.setMessage("Unknown gameplay command. Use movement, r to restart, or q to quit.");
    }
}

// Names the victory phase.
std::string WonState::getName() const {
    return "Won";
}

// Shows the victory restart/quit prompt.
void WonState::onEnter(Game &game) {
    game.setMessage(
        "You won! Final score: " + std::to_string(game.getFinalScore()) +
        ". Enter r to play again or q to quit.");
}

// Limits post-win commands to restart or quit.
void WonState::handleCommand(Game &game, const std::string &command) {
    if (command == "r") {
        game.restart();
    } else if (command == "q") {
        game.quit();
    } else {
        game.setMessage("Run complete. Enter r to restart or q to quit.");
    }
}

// Names the defeat phase.
std::string LostState::getName() const {
    return "Lost";
}

// Shows the defeat restart/quit prompt.
void LostState::onEnter(Game &game) {
    game.setMessage("You lost. Enter r to play again or q to quit.");
}

// Limits post-loss commands to restart or quit.
void LostState::handleCommand(Game &game, const std::string &command) {
    if (command == "r") {
        game.restart();
    } else if (command == "q") {
        game.quit();
    } else {
        game.setMessage("Run over. Enter r to restart or q to quit.");
    }
}
