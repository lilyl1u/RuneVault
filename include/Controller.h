#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <iosfwd>

#include "Game.h"

// ==================== DESIGN PATTERN: MVC - Controller ====================
// Controller reads user input and delegates the meaning of commands to Game.
// This keeps terminal input separate from model rules and display rendering.
class Controller {
    Game &game;

  public:
    // Stores a non-owning reference to the Game model.
    explicit Controller(Game &game);

    // Runs the read-command, update-model, redraw loop.
    void run(std::istream &in, std::ostream &out);
};

#endif
