#ifndef TEXTDISPLAY_H
#define TEXTDISPLAY_H

#include <iosfwd>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Position.h"
#include "VaultLevel.h"

// ==================== DESIGN PATTERN: MVC - View ====================
// TextDisplay is the View part of MVC.
// It only knows how to draw the current model state; it should not decide game rules.
class TextDisplay {
  public:
    // Draws the full RuneVault screen: 25 rows of map and 5 rows of status.
    void draw(
        std::ostream &out,
        const VaultLevel &level,
        const std::string &status,
        const std::string &message,
        const std::optional<Position> &arcanistPosition = std::nullopt,
        const std::vector<std::pair<Position, char>> &items = {},
        const std::vector<std::pair<Position, char>> &spectres = {}) const;
};

#endif
