#include "VaultLevel.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <optional>
#include <stdexcept>

class VaultMovementContext: public MovementContext {
    const VaultLevel &level;
    const Position &arcanistPosition;

  public:
    // Adapts VaultLevel movement rules to the MovementContext Strategy interface.
    VaultMovementContext(const VaultLevel &level, const Position &arcanistPosition):
        level{level},
        arcanistPosition{arcanistPosition} {}

    // Delegates legal-move checks back to VaultLevel.
    bool canSpectreMoveTo(
        const AbstractSpectre &spectre,
        const Position &position) const override {
        return level.canSpectreMoveTo(spectre, position, arcanistPosition);
    }

    // BONUS FEATURE: Spectre intelligence
    // Provides the BFS target without exposing VaultLevel internals to strategies.
    Position getArcanistPosition() const override {
        return arcanistPosition;
    }

    // BONUS FEATURE: Spectre intelligence
    // Lets pathfinding code perform bounds checks through the MovementContext abstraction.
    bool inBounds(const Position &position) const override {
        return level.getMap().inBounds(position);
    }

    // BONUS FEATURE: Spectre intelligence
    // Adapts VaultLevel traversal rules for intelligent cross-chamber pursuit.
    bool canSpectrePathThrough(
        const AbstractSpectre &spectre,
        const Position &position) const override {
        return level.canSpectrePathThrough(spectre, position);
    }
};

class VaultHostilityContext: public HostilityContext {
    const VaultLevel &level;
    const Position &arcanistPosition;

  public:
    // Adapts current level facts to the HostilityContext Strategy interface.
    VaultHostilityContext(const VaultLevel &level, const Position &arcanistPosition):
        level{level},
        arcanistPosition{arcanistPosition} {}

    // Returns the Arcanist position for adjacency checks.
    Position getArcanistPosition() const override {
        return arcanistPosition;
    }

    // Returns whether the Arcanist is adjacent to this anchor's guarded item.
    bool isArcanistAdjacentToGuardedItem(const Position &anchorPosition) const override {
        return level.isArcanistAdjacentToGuardedItem(anchorPosition, arcanistPosition);
    }
};

// Creates a level using the default terrain map and no generated items yet.
VaultLevel::VaultLevel(int levelNumber):
    levelNumber{levelNumber},
    map{Map::createDefault()},
    items{},
    spectres{},
    guardedItems{},
    hiddenStairway{0, 0},
    stairwayVisible{false} {}

// Creates a level using caller-supplied terrain from layout parsing.
VaultLevel::VaultLevel(int levelNumber, const Map &map):
    levelNumber{levelNumber},
    map{map},
    items{},
    spectres{},
    guardedItems{},
    hiddenStairway{0, 0},
    stairwayVisible{false} {}

// Returns the current vault level number.
int VaultLevel::getLevelNumber() const {
    return levelNumber;
}

// Returns read-only access to the terrain map.
const Map &VaultLevel::getMap() const {
    return map;
}

// Returns the hidden stairway location chosen during item generation.
Position VaultLevel::getHiddenStairway() const {
    return hiddenStairway;
}

// Reports whether the stairway should be drawn and used.
bool VaultLevel::isStairwayVisible() const {
    return stairwayVisible;
}

// Spawns the Step 6 item set while avoiding the Arcanist and hidden stairway.
void VaultLevel::generateBasicItems(
    Random &rng,
    const Position &arcanistPosition,
    bool spawnAegisCloak) {
    items.clear();
    spectres.clear();
    guardedItems.clear();
    stairwayVisible = false;

    ItemFactory factory;
    std::vector<Position> blocked{arcanistPosition};

    hiddenStairway = randomFreeSpawnTile(rng, blocked);
    blocked.emplace_back(hiddenStairway);

    for (int i = 0; i < 10; ++i) {
        Position pos = randomFreeSpawnTile(rng, blocked);
        blocked.emplace_back(pos);
        ScrollType type = static_cast<ScrollType>(rng.range(0, 5));
        addItem(factory.createScroll(type, pos));
    }

    for (int i = 0; i < 9; ++i) {
        Position pos = randomFreeSpawnTile(rng, blocked);
        blocked.emplace_back(pos);

        FragmentType type = FragmentType::CommonShard;
        if (rng.chance(2, 7)) {
            type = FragmentType::ResonantShard;
        }

        addItem(factory.createFragment(type, pos));
    }

    Position cofferPosition = randomFreeSpawnTile(rng, blocked);
    blocked.emplace_back(cofferPosition);
    Position cofferAnchorPosition = randomAdjacentFreeSpawnTile(rng, cofferPosition, blocked);
    blocked.emplace_back(cofferAnchorPosition);
    addGuardedItem(factory.createFragment(FragmentType::ShardCoffer, cofferPosition), cofferAnchorPosition);

    if (spawnAegisCloak) {
        Position aegisPosition = randomFreeSpawnTile(rng, blocked);
        blocked.emplace_back(aegisPosition);
        Position aegisAnchorPosition = randomAdjacentFreeSpawnTile(rng, aegisPosition, blocked);
        blocked.emplace_back(aegisAnchorPosition);
        addGuardedItem(factory.createMajorItem(MajorItemType::AegisCloak, aegisPosition), aegisAnchorPosition);
    }
}

// Spawns 20 non-anchor spectres while avoiding the Arcanist, stairway, items, and other spectres.
void VaultLevel::generateSpectres(
    Random &rng,
    const Position &arcanistPosition,
    bool enableSpecterLordChase) {
    SpectreFactory factory;
    std::vector<Position> blocked{arcanistPosition, hiddenStairway};

    for (const auto &item: items) {
        blocked.emplace_back(item->getPosition());
    }
    for (const auto &spectre: spectres) {
        blocked.emplace_back(spectre->getPosition());
    }

    for (int i = 0; i < 20; ++i) {
        Position pos = randomFreeSpectreSpawnTile(rng, blocked);
        blocked.emplace_back(pos);
        int chamberId = map.getCell(pos).getChamberId();
        addSpectre(factory.createRandomNonAnchor(rng, pos, chamberId, enableSpecterLordChase));
    }

    assignCipherGemCarrier(rng);
}

// Loads digit-coded items from a layout file and chooses a hidden stairway.
void VaultLevel::loadItemsFromLayoutRows(
    Random &rng,
    const std::vector<std::string> &rows,
    const Position &arcanistPosition) {
    if (static_cast<int>(rows.size()) != Map::Height) {
        throw std::invalid_argument{"Layout file must contain exactly 25 rows"};
    }

    items.clear();
    spectres.clear();
    guardedItems.clear();
    stairwayVisible = false;

    ItemFactory factory;
    std::vector<Position> blocked{arcanistPosition};
    std::vector<Position> shardCofferPositions;

    for (int row = 0; row < Map::Height; ++row) {
        if (static_cast<int>(rows[row].length()) != Map::Width) {
            throw std::invalid_argument{"Each layout row must contain exactly 79 characters"};
        }

        for (int col = 0; col < Map::Width; ++col) {
            unsigned char symbol = static_cast<unsigned char>(rows[row][col]);
            if (!std::isdigit(symbol)) {
                continue;
            }

            Position pos{row, col};
            if (!map.getCell(pos).isSpawnFloor()) {
                throw std::invalid_argument{"Layout item digits must be on chamber floor tiles"};
            }
            if (pos == arcanistPosition) {
                throw std::invalid_argument{"Layout cannot place an item on the Arcanist spawn"};
            }

            int digit = rows[row][col] - '0';
            if (digit <= 5) {
                addItem(factory.createScroll(static_cast<ScrollType>(digit), pos));
            } else if (digit == 9) {
                shardCofferPositions.emplace_back(pos);
            } else {
                FragmentType type = FragmentType::CommonShard;
                if (digit == 7) {
                    type = FragmentType::ResonantShard;
                } else if (digit == 8) {
                    type = FragmentType::LichCache;
                }
                addItem(factory.createFragment(type, pos));
            }

            blocked.emplace_back(pos);
        }
    }

    for (const Position &cofferPosition: shardCofferPositions) {
        Position anchorPosition = randomAdjacentFreeSpawnTile(rng, cofferPosition, blocked);
        blocked.emplace_back(anchorPosition);
        addGuardedItem(
            factory.createFragment(FragmentType::ShardCoffer, cofferPosition),
            anchorPosition);
    }

    hiddenStairway = randomFreeSpawnTile(rng, blocked);
}

// Validates an item position and transfers ownership into this level.
void VaultLevel::addItem(std::unique_ptr<Item> item) {
    if (!item) {
        throw std::invalid_argument{"Cannot add a null item"};
    }
    if (!map.inBounds(item->getPosition()) ||
        !map.getCell(item->getPosition()).isSpawnFloor()) {
        throw std::invalid_argument{"Item must be placed on a chamber floor tile"};
    }
    if (hasItemAt(item->getPosition())) {
        throw std::invalid_argument{"Cannot place two items on the same tile"};
    }
    if (hasSpectreAt(item->getPosition())) {
        throw std::invalid_argument{"Cannot place an item on a spectre"};
    }

    // Ownership moves into the level; RAII cleans it up when the level is destroyed.
    items.emplace_back(std::move(item));
}

// Validates a spectre position and transfers ownership into this level.
void VaultLevel::addSpectre(std::unique_ptr<AbstractSpectre> spectre) {
    if (!spectre) {
        throw std::invalid_argument{"Cannot add a null spectre"};
    }
    if (!map.inBounds(spectre->getPosition()) ||
        !map.getCell(spectre->getPosition()).isSpawnFloor()) {
        throw std::invalid_argument{"Spectre must be placed on a chamber floor tile"};
    }
    if (hasItemAt(spectre->getPosition())) {
        throw std::invalid_argument{"Cannot place a spectre on an item"};
    }
    if (hasSpectreAt(spectre->getPosition())) {
        throw std::invalid_argument{"Cannot place two spectres on the same tile"};
    }

    // Ownership moves into the level; RAII cleans it up when the level is destroyed.
    spectres.emplace_back(std::move(spectre));
}

// Places an item and a Vault Anchor as one guarded pair.
void VaultLevel::addGuardedItem(std::unique_ptr<Item> item, const Position &anchorPosition) {
    if (!item) {
        throw std::invalid_argument{"Cannot add a null guarded item"};
    }
    if (!map.inBounds(anchorPosition) || !map.getCell(anchorPosition).isSpawnFloor()) {
        throw std::invalid_argument{"Guard anchor must be on a chamber floor tile"};
    }
    if (hasItemAt(anchorPosition) || hasSpectreAt(anchorPosition)) {
        throw std::invalid_argument{"Guard anchor tile is occupied"};
    }

    Position itemPosition = item->getPosition();
    addItem(std::move(item));

    SpectreFactory factory;
    int chamberId = map.getCell(anchorPosition).getChamberId();
    addSpectre(factory.create(SpectreType::VaultAnchor, anchorPosition, chamberId));
    guardedItems.push_back(GuardedItem{itemPosition, anchorPosition});
}

// Looks for an item at a position without allowing mutation.
const Item *VaultLevel::getItemAt(const Position &position) const {
    // Lambda used locally with find_if: short, readable, and avoids exposing item storage.
    auto found = std::find_if(items.begin(), items.end(), [&position](const auto &item) {
        return item->getPosition() == position;
    });

    if (found == items.end()) {
        return nullptr;
    }

    return found->get();
}

// Looks for an item at a position and allows mutation without transferring ownership.
Item *VaultLevel::getItemAt(const Position &position) {
    auto found = std::find_if(items.begin(), items.end(), [&position](const auto &item) {
        return item->getPosition() == position;
    });

    if (found == items.end()) {
        return nullptr;
    }

    return found->get();
}

// Looks for a spectre at a position without allowing mutation.
const AbstractSpectre *VaultLevel::getSpectreAt(const Position &position) const {
    // Lambda used locally with find_if: short, readable, and avoids exposing storage.
    auto found = std::find_if(spectres.begin(), spectres.end(), [&position](const auto &spectre) {
        return spectre->getPosition() == position;
    });

    if (found == spectres.end()) {
        return nullptr;
    }

    return found->get();
}

// Looks for a spectre at a position and allows mutation without transferring ownership.
AbstractSpectre *VaultLevel::getSpectreAt(const Position &position) {
    auto found = std::find_if(spectres.begin(), spectres.end(), [&position](const auto &spectre) {
        return spectre->getPosition() == position;
    });

    if (found == spectres.end()) {
        return nullptr;
    }

    return found->get();
}

// Removes an item from the level and returns ownership to the caller.
std::unique_ptr<Item> VaultLevel::removeItemAt(const Position &position) {
    auto found = std::find_if(items.begin(), items.end(), [&position](const auto &item) {
        return item->getPosition() == position;
    });

    if (found == items.end()) {
        return nullptr;
    }

    // Move the unique_ptr out before erasing, so the caller receives ownership.
    std::unique_ptr<Item> removed = std::move(*found);
    items.erase(found);
    return removed;
}

// Removes a spectre from the level and returns ownership to the caller.
std::unique_ptr<AbstractSpectre> VaultLevel::removeSpectreAt(const Position &position) {
    auto found = std::find_if(spectres.begin(), spectres.end(), [&position](const auto &spectre) {
        return spectre->getPosition() == position;
    });

    if (found == spectres.end()) {
        return nullptr;
    }

    // Move the unique_ptr out before erasing, so the caller receives ownership.
    std::unique_ptr<AbstractSpectre> removed = std::move(*found);
    spectres.erase(found);
    return removed;
}

// Moves spectres in row-major order, with each original spectre moving at most once.
void VaultLevel::moveSpectres(Random &rng, const Position &arcanistPosition) {
    std::vector<Position> order;

    for (const auto &spectre: spectres) {
        order.emplace_back(spectre->getPosition());
    }

    std::sort(order.begin(), order.end(), [](const Position &lhs, const Position &rhs) {
        if (lhs.row != rhs.row) {
            return lhs.row < rhs.row;
        }
        return lhs.col < rhs.col;
    });

    VaultMovementContext context{*this, arcanistPosition};

    for (const Position &originalPosition: order) {
        AbstractSpectre *spectre = getSpectreAt(originalPosition);
        if (!spectre) {
            continue;
        }

        std::optional<Position> nextPosition = spectre->chooseMove(context, rng);
        if (nextPosition) {
            spectre->setPosition(nextPosition.value());
        }
    }
}

// Resolves spectre turns in row-major order and returns hostile attackers.
std::vector<AbstractSpectre *> VaultLevel::takeSpectreTurns(
    Random &rng,
    const Position &arcanistPosition) {
    std::vector<Position> order;
    std::vector<AbstractSpectre *> attackers;

    for (const auto &spectre: spectres) {
        order.emplace_back(spectre->getPosition());
    }

    std::sort(order.begin(), order.end(), [](const Position &lhs, const Position &rhs) {
        if (lhs.row != rhs.row) {
            return lhs.row < rhs.row;
        }
        return lhs.col < rhs.col;
    });

    VaultMovementContext movementContext{*this, arcanistPosition};
    VaultHostilityContext hostilityContext{*this, arcanistPosition};

    for (const Position &originalPosition: order) {
        AbstractSpectre *spectre = getSpectreAt(originalPosition);
        if (!spectre) {
            continue;
        }

        if (spectre->isHostile(hostilityContext)) {
            attackers.emplace_back(spectre);
            continue;
        }

        std::optional<Position> nextPosition = spectre->chooseMove(movementContext, rng);
        if (nextPosition) {
            spectre->setPosition(nextPosition.value());
        }
    }

    return attackers;
}

// Checks terrain/chamber/occupancy rules for spectre movement.
bool VaultLevel::canSpectreMoveTo(
    const AbstractSpectre &spectre,
    const Position &position,
    const Position &arcanistPosition) const {
    if (!map.inBounds(position)) {
        return false;
    }
    if (position == arcanistPosition) {
        return false;
    }

    const Cell &cell = map.getCell(position);
    if (!cell.isSpawnFloor()) {
        return false;
    }
    if (cell.getChamberId() != spectre.getSpawnChamberId()) {
        return false;
    }
    if (hasItemAt(position)) {
        return false;
    }
    if (hasSpectreAt(position)) {
        return false;
    }

    return true;
}

// BONUS FEATURE: Spectre intelligence
// Path traversal allows corridors/archways, while normal random movement stays chamber-local.
bool VaultLevel::canSpectrePathThrough(
    const AbstractSpectre &spectre,
    const Position &position) const {
    (void) spectre;

    if (!map.inBounds(position)) {
        return false;
    }

    const Cell &cell = map.getCell(position);
    if (!cell.isWalkable()) {
        return false;
    }
    if (hasItemAt(position)) {
        return false;
    }
    if (hasSpectreAt(position)) {
        return false;
    }

    return true;
}

// Returns true when a guarded item is still protected by its anchor.
bool VaultLevel::isGuardedItemLocked(const Position &position) const {
    for (const GuardedItem &guardedItem: guardedItems) {
        if (guardedItem.itemPosition == position && getSpectreAt(guardedItem.anchorPosition)) {
            return true;
        }
    }

    return false;
}

// Returns true when the Arcanist is adjacent to the item guarded by a specific anchor.
bool VaultLevel::isArcanistAdjacentToGuardedItem(
    const Position &anchorPosition,
    const Position &arcanistPosition) const {
    for (const GuardedItem &guardedItem: guardedItems) {
        if (guardedItem.anchorPosition == anchorPosition &&
            std::abs(guardedItem.itemPosition.row - arcanistPosition.row) <= 1 &&
            std::abs(guardedItem.itemPosition.col - arcanistPosition.col) <= 1) {
            return true;
        }
    }

    return false;
}

// Assigns the Cipher Gem to one random non-anchor spectre.
void VaultLevel::assignCipherGemCarrier(Random &rng) {
    std::vector<AbstractSpectre *> candidates;

    for (const auto &spectre: spectres) {
        spectre->setCipherGemCarrier(false);
        if (spectre->getType() != SpectreType::VaultAnchor) {
            candidates.emplace_back(spectre.get());
        }
    }

    if (candidates.empty()) {
        return;
    }

    candidates[rng.range(0, static_cast<int>(candidates.size()) - 1)]->setCipherGemCarrier(true);
}

// Reveals the hidden stairway.
void VaultLevel::revealStairway() {
    stairwayVisible = true;
}

// Counts how many unique_ptr-owned items are still in the level.
int VaultLevel::getItemCount() const {
    return static_cast<int>(items.size());
}

// Counts how many unique_ptr-owned spectres are still in the level.
int VaultLevel::getSpectreCount() const {
    return static_cast<int>(spectres.size());
}

// Counts guarded item/anchor pairs.
int VaultLevel::getGuardedItemCount() const {
    return static_cast<int>(guardedItems.size());
}

// Returns the Cipher Gem carrier position, if a carrier exists.
std::optional<Position> VaultLevel::getCipherCarrierPosition() const {
    for (const auto &spectre: spectres) {
        if (spectre->hasCipherGem()) {
            return spectre->getPosition();
        }
    }

    return std::nullopt;
}

// Converts owned items into simple render data for TextDisplay.
std::vector<std::pair<Position, char>> VaultLevel::getItemRenderObjects() const {
    std::vector<std::pair<Position, char>> renderObjects;

    for (const auto &item: items) {
        renderObjects.emplace_back(item->getPosition(), item->getSymbol());
    }

    return renderObjects;
}

// Converts owned spectres into simple render data for TextDisplay.
std::vector<std::pair<Position, char>> VaultLevel::getSpectreRenderObjects() const {
    std::vector<std::pair<Position, char>> renderObjects;

    for (const auto &spectre: spectres) {
        renderObjects.emplace_back(spectre->getPosition(), spectre->getSymbol());
    }

    return renderObjects;
}

// Checks whether an item already occupies a position.
bool VaultLevel::hasItemAt(const Position &position) const {
    return getItemAt(position) != nullptr;
}

// Checks whether a spectre already occupies a position.
bool VaultLevel::hasSpectreAt(const Position &position) const {
    return getSpectreAt(position) != nullptr;
}

// Chooses a legal floor tile that is not blocked and has no item.
Position VaultLevel::randomFreeSpawnTile(
    Random &rng,
    const std::vector<Position> &blocked) const {
    std::vector<Position> choices;

    for (int chamberId = 1; chamberId <= 5; ++chamberId) {
        std::vector<Position> chamberTiles = map.getSpawnTiles(chamberId);
        for (const Position &pos: chamberTiles) {
            bool isBlocked = std::find(blocked.begin(), blocked.end(), pos) != blocked.end();
            if (!isBlocked && !hasItemAt(pos)) {
                choices.emplace_back(pos);
            }
        }
    }

    if (choices.empty()) {
        throw std::logic_error{"No free item spawn tiles exist"};
    }

    return choices[rng.range(0, static_cast<int>(choices.size()) - 1)];
}

// Chooses a legal spectre spawn tile that is not blocked, item-occupied, or spectre-occupied.
Position VaultLevel::randomFreeSpectreSpawnTile(
    Random &rng,
    const std::vector<Position> &blocked) const {
    std::vector<Position> choices;

    for (int chamberId = 1; chamberId <= 5; ++chamberId) {
        std::vector<Position> chamberTiles = map.getSpawnTiles(chamberId);
        for (const Position &pos: chamberTiles) {
            bool isBlocked = std::find(blocked.begin(), blocked.end(), pos) != blocked.end();
            if (!isBlocked && !hasItemAt(pos) && !hasSpectreAt(pos)) {
                choices.emplace_back(pos);
            }
        }
    }

    if (choices.empty()) {
        throw std::logic_error{"No free spectre spawn tiles exist"};
    }

    return choices[rng.range(0, static_cast<int>(choices.size()) - 1)];
}

// Chooses a legal floor tile within one-block radius of a guarded item.
Position VaultLevel::randomAdjacentFreeSpawnTile(
    Random &rng,
    const Position &itemPosition,
    const std::vector<Position> &blocked) const {
    std::vector<Position> choices;

    for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
        for (int colOffset = -1; colOffset <= 1; ++colOffset) {
            if (rowOffset == 0 && colOffset == 0) {
                continue;
            }

            Position candidate{itemPosition.row + rowOffset, itemPosition.col + colOffset};
            bool isBlocked = std::find(blocked.begin(), blocked.end(), candidate) != blocked.end();
            if (map.inBounds(candidate) &&
                map.getCell(candidate).isSpawnFloor() &&
                !isBlocked &&
                !hasItemAt(candidate) &&
                !hasSpectreAt(candidate)) {
                choices.emplace_back(candidate);
            }
        }
    }

    if (choices.empty()) {
        throw std::logic_error{"No free adjacent guard spawn tile exists"};
    }

    return choices[rng.range(0, static_cast<int>(choices.size()) - 1)];
}
