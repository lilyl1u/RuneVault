#ifndef SPECTREFACTORY_H
#define SPECTREFACTORY_H

#include <memory>

#include "Random.h"
#include "Spectre.h"

// ==================== DESIGN PATTERN: Factory Method / Simple Factory ====================
// SpectreFactory centralizes enemy construction and random type selection.
// Game/VaultLevel depend on AbstractSpectre instead of concrete enemy classes.
class SpectreFactory {
  public:
    // Creates a specific spectre type and transfers ownership to the caller.
    std::unique_ptr<AbstractSpectre> create(
        SpectreType type,
        const Position &position,
        int spawnChamberId) const;

    // Creates one random non-anchor spectre using the required spawn probabilities.
    std::unique_ptr<AbstractSpectre> createRandomNonAnchor(
        Random &rng,
        const Position &position,
        int spawnChamberId) const;
};

#endif
