#include "ArcanistFactory.h"

#include <stdexcept>

// Creates the concrete Arcanist class requested by the class-selection command.
std::unique_ptr<AbstractArcanist> ArcanistFactory::create(
    char classCode,
    const Position &position) const {
    if (classCode == 's') {
        return std::make_unique<Sage>(position);
    }
    if (classCode == 'h') {
        return std::make_unique<Hexblade>(position);
    }
    if (classCode == 'w') {
        return std::make_unique<Warden>(position);
    }
    if (classCode == 'v') {
        return std::make_unique<Voidwalker>(position);
    }

    throw std::invalid_argument{"Unknown Arcanist class code"};
}
