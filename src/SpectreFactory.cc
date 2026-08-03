#include "SpectreFactory.h"

#include <stdexcept>

// Creates a concrete spectre while returning it through the abstract interface.
std::unique_ptr<AbstractSpectre> SpectreFactory::create(
    SpectreType type,
    const Position &position,
    int spawnChamberId) const {
    if (type == SpectreType::Wraith) {
        return std::make_unique<Wraith>(position, spawnChamberId);
    }
    if (type == SpectreType::Banshee) {
        return std::make_unique<Banshee>(position, spawnChamberId);
    }
    if (type == SpectreType::Revenant) {
        return std::make_unique<Revenant>(position, spawnChamberId);
    }
    if (type == SpectreType::Shade) {
        return std::make_unique<Shade>(position, spawnChamberId);
    }
    if (type == SpectreType::Lich) {
        return std::make_unique<Lich>(position, spawnChamberId);
    }
    if (type == SpectreType::VaultAnchor) {
        return std::make_unique<VaultAnchor>(position, spawnChamberId);
    }
    if (type == SpectreType::SpecterLord) {
        return std::make_unique<SpecterLord>(position, spawnChamberId);
    }

    throw std::invalid_argument{"Unknown spectre type"};
}

// Chooses a non-anchor spectre according to the spec's 18-part probability table.
std::unique_ptr<AbstractSpectre> SpectreFactory::createRandomNonAnchor(
    Random &rng,
    const Position &position,
    int spawnChamberId) const {
    int roll = rng.range(1, 18);

    if (roll <= 4) {
        return create(SpectreType::Wraith, position, spawnChamberId);
    }
    if (roll <= 7) {
        return create(SpectreType::Banshee, position, spawnChamberId);
    }
    if (roll <= 12) {
        return create(SpectreType::Shade, position, spawnChamberId);
    }
    if (roll <= 14) {
        return create(SpectreType::Revenant, position, spawnChamberId);
    }
    if (roll <= 16) {
        return create(SpectreType::SpecterLord, position, spawnChamberId);
    }

    return create(SpectreType::Lich, position, spawnChamberId);
}
