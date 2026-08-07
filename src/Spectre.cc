#include "Spectre.h"

#include <algorithm>

// Stores shared spectre data from the specification.
AbstractSpectre::AbstractSpectre(
    SpectreType type,
    const std::string &name,
    char symbol,
    int focusPoints,
    int power,
    int ward,
    const Position &position,
    int spawnChamberId,
    std::unique_ptr<MovementStrategy> movementStrategy,
    std::unique_ptr<HostilityStrategy> hostilityStrategy):
    type{type},
    name{name},
    symbol{symbol},
    focusPoints{focusPoints},
    power{power},
    ward{ward},
    position{position},
    spawnChamberId{spawnChamberId},
    movementStrategy{std::move(movementStrategy)},
    hostilityStrategy{std::move(hostilityStrategy)},
    cipherGemCarrier{false} {}

// Returns the concrete spectre type.
SpectreType AbstractSpectre::getType() const {
    return type;
}

// Returns the spectre name for messages/tests.
std::string AbstractSpectre::getName() const {
    return name;
}

// Returns the spectre display symbol.
char AbstractSpectre::getSymbol() const {
    return symbol;
}

// Returns current Focus Points.
int AbstractSpectre::getFocusPoints() const {
    return focusPoints;
}

// Returns whether this spectre has no FP remaining.
bool AbstractSpectre::isDefeated() const {
    return focusPoints == 0;
}

// Returns attack Power.
int AbstractSpectre::getPower() const {
    return power;
}

// Returns Ward Rating.
int AbstractSpectre::getWard() const {
    return ward;
}

// Returns current board position.
Position AbstractSpectre::getPosition() const {
    return position;
}

// Returns the chamber where this spectre spawned.
int AbstractSpectre::getSpawnChamberId() const {
    return spawnChamberId;
}

// Returns the movement strategy name for tests/report discussion.
std::string AbstractSpectre::getMovementStrategyName() const {
    return movementStrategy->getName();
}

// Returns the hostility strategy name for tests/report discussion.
std::string AbstractSpectre::getHostilityStrategyName() const {
    return hostilityStrategy->getName();
}

// Returns whether this spectre carries the Cipher Gem.
bool AbstractSpectre::hasCipherGem() const {
    return cipherGemCarrier;
}

// Delegates movement choice to the owned MovementStrategy.
std::optional<Position> AbstractSpectre::chooseMove(
    const MovementContext &context,
    Random &rng) const {
    // ==================== DESIGN PATTERN: Strategy ====================
    // Spectre does not know whether it is random-moving or stationary; it asks its strategy.
    return movementStrategy->chooseMove(*this, context, rng);
}

// Delegates hostility decision to the owned HostilityStrategy.
bool AbstractSpectre::isHostile(const HostilityContext &context) const {
    // ==================== DESIGN PATTERN: Strategy ====================
    // Spectre does not check concrete types; it asks its hostility strategy.
    return hostilityStrategy->isHostile(*this, context);
}

// Updates position after movement logic validates the target tile.
void AbstractSpectre::setPosition(const Position &newPosition) {
    position = newPosition;
}

// Applies damage while keeping FP from dropping below 0.
void AbstractSpectre::takeDamage(int damage) {
    focusPoints = std::max(0, focusPoints - damage);
}

// Marks this spectre as the Cipher Gem carrier or clears that state.
void AbstractSpectre::setCipherGemCarrier(bool carriesCipherGem) {
    cipherGemCarrier = carriesCipherGem;
}

// Creates a Wraith.
Wraith::Wraith(const Position &position, int spawnChamberId):
    AbstractSpectre{
        SpectreType::Wraith,
        "Wraith",
        'W',
        60,
        20,
        10,
        position,
        spawnChamberId,
        std::make_unique<RandomMovement>(),
        std::make_unique<AlwaysHostile>()} {}

// Creates a Banshee.
Banshee::Banshee(const Position &position, int spawnChamberId):
    AbstractSpectre{
        SpectreType::Banshee,
        "Banshee",
        'B',
        40,
        30,
        5,
        position,
        spawnChamberId,
        std::make_unique<RandomMovement>(),
        std::make_unique<AlwaysHostile>()} {}

// Creates a Revenant.
Revenant::Revenant(const Position &position, int spawnChamberId):
    AbstractSpectre{
        SpectreType::Revenant,
        "Revenant",
        'R',
        100,
        18,
        20,
        position,
        spawnChamberId,
        std::make_unique<RandomMovement>(),
        std::make_unique<AlwaysHostile>()} {}

// Creates a Shade.
Shade::Shade(const Position &position, int spawnChamberId):
    AbstractSpectre{
        SpectreType::Shade,
        "Shade",
        'S',
        50,
        8,
        8,
        position,
        spawnChamberId,
        std::make_unique<RandomMovement>(),
        std::make_unique<AlwaysHostile>()} {}

// Creates a Lich.
Lich::Lich(const Position &position, int spawnChamberId):
    AbstractSpectre{
        SpectreType::Lich,
        "Lich",
        'L',
        30,
        40,
        5,
        position,
        spawnChamberId,
        std::make_unique<RandomMovement>(),
        std::make_unique<NeutralHostility>()} {}

// Creates a Vault Anchor.
VaultAnchor::VaultAnchor(const Position &position, int spawnChamberId):
    AbstractSpectre{
        SpectreType::VaultAnchor,
        "Vault Anchor",
        'A',
        120,
        22,
        18,
        position,
        spawnChamberId,
        std::make_unique<StationaryMovement>(),
        std::make_unique<GuardedItemHostility>()} {}

// Creates a Specter Lord.
SpecterLord::SpecterLord(const Position &position, int spawnChamberId):
    AbstractSpectre{
        SpectreType::SpecterLord,
        "Specter Lord",
        'X',
        80,
        28,
        15,
        position,
        spawnChamberId,
        std::make_unique<ChaseMovement>(),
        std::make_unique<AlwaysHostile>()} {}
