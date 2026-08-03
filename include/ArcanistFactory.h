#ifndef ARCANISTFACTORY_H
#define ARCANISTFACTORY_H

#include <memory>

#include "Arcanist.h"

// ==================== DESIGN PATTERN: Factory Method / Simple Factory ====================
// The factory centralizes Arcanist creation so Game does not need to know the
// constructor details for Sage, Hexblade, Warden, and Voidwalker.
class ArcanistFactory {
  public:
    // Creates the chosen Arcanist and transfers ownership to the caller.
    std::unique_ptr<AbstractArcanist> create(char classCode, const Position &position) const;
};

#endif
