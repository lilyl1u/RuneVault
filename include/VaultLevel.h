#ifndef VAULTLEVEL_H
#define VAULTLEVEL_H

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Item.h"
#include "ItemFactory.h"
#include "Map.h"
#include "Random.h"
#include "Spectre.h"
#include "SpectreFactory.h"

// ==================== DESIGN PATTERN: MVC - Model ====================
// VaultLevel is part of the Model.
// It stores level data and owns the items/spectres for the current level.
class VaultLevel {
    struct GuardedItem {
        Position itemPosition;
        Position anchorPosition;
    };

    int levelNumber;
    Map map;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::unique_ptr<AbstractSpectre>> spectres;
    std::vector<GuardedItem> guardedItems;
    Position hiddenStairway;
    bool stairwayVisible;

    // Returns true when an item already occupies the given position.
    bool hasItemAt(const Position &position) const;

    // Returns true when a spectre already occupies the given position.
    bool hasSpectreAt(const Position &position) const;

    // Chooses a legal item spawn tile that is not blocked or occupied.
    Position randomFreeSpawnTile(Random &rng, const std::vector<Position> &blocked) const;

    // Chooses a legal spectre spawn tile that is not blocked or occupied.
    Position randomFreeSpectreSpawnTile(Random &rng, const std::vector<Position> &blocked) const;

    // Chooses an adjacent legal anchor tile for a guarded item.
    Position randomAdjacentFreeSpawnTile(Random &rng, const Position &itemPosition, const std::vector<Position> &blocked) const;

  public:
    // Creates a vault level with the default map layout.
    explicit VaultLevel(int levelNumber = 1);

    // Creates a vault level with a parsed layout map.
    VaultLevel(int levelNumber, const Map &map);

    // Returns this level's number in the run.
    int getLevelNumber() const;

    // Returns the terrain map.
    const Map &getMap() const;

    // Returns the hidden stairway tile chosen during generation.
    Position getHiddenStairway() const;

    // Returns whether the stairway should currently be drawn.
    bool isStairwayVisible() const;

    // ==================== DESIGN PATTERN: Factory Method / Simple Factory ====================
    // Level generation uses ItemFactory instead of calling concrete item constructors directly.
    // Spawns scrolls, fragments, guarded Shard Coffer, optional Aegis, and hidden stairway.
    void generateBasicItems(Random &rng, const Position &arcanistPosition, bool spawnAegisCloak = false);

    // Spawns the Step 9 spectres after item generation.
    void generateSpectres(
        Random &rng,
        const Position &arcanistPosition,
        bool enableSpecterLordChase = false,
        bool lichesHostile = false);

    // Loads item positions from digit-coded layout rows.
    void loadItemsFromLayoutRows(
        Random &rng,
        const std::vector<std::string> &rows,
        const Position &arcanistPosition);

    // Transfers ownership of an item into the level.
    void addItem(std::unique_ptr<Item> item);

    // Transfers ownership of a spectre into the level.
    void addSpectre(std::unique_ptr<AbstractSpectre> spectre);

    // Places a guarded item and its Vault Anchor together.
    void addGuardedItem(std::unique_ptr<Item> item, const Position &anchorPosition);

    // Finds an item at a position without transferring ownership.
    const Item *getItemAt(const Position &position) const;

    // Finds a mutable item at a position without transferring ownership.
    Item *getItemAt(const Position &position);

    // Finds a spectre at a position without transferring ownership.
    const AbstractSpectre *getSpectreAt(const Position &position) const;

    // Finds a mutable spectre at a position without transferring ownership.
    AbstractSpectre *getSpectreAt(const Position &position);

    // Removes an item and transfers ownership to the caller.
    std::unique_ptr<Item> removeItemAt(const Position &position);

    // Removes a spectre and transfers ownership to the caller.
    std::unique_ptr<AbstractSpectre> removeSpectreAt(const Position &position);

    // Moves all spectres once in row-major order.
    void moveSpectres(Random &rng, const Position &arcanistPosition);

    // Resolves one spectre turn pass and returns spectres that should attack.
    std::vector<AbstractSpectre *> takeSpectreTurns(Random &rng, const Position &arcanistPosition);

    // Returns whether a spectre could legally move to a position.
    bool canSpectreMoveTo(const AbstractSpectre &spectre, const Position &position, const Position &arcanistPosition) const;

    // BONUS FEATURE: Spectre intelligence
    // Returns whether pathfinding spectres may plan through this walkable tile.
    bool canSpectrePathThrough(const AbstractSpectre &spectre, const Position &position) const;

    // Returns true when a guarded item is still protected by its anchor.
    bool isGuardedItemLocked(const Position &position) const;

    // Returns true when the Arcanist is adjacent to an item guarded by this anchor.
    bool isArcanistAdjacentToGuardedItem(const Position &anchorPosition, const Position &arcanistPosition) const;

    // Assigns the Cipher Gem to one random non-anchor spectre.
    void assignCipherGemCarrier(Random &rng);

    // Reveals the hidden stairway.
    void revealStairway();

    // Returns the number of items currently owned by the level.
    int getItemCount() const;

    // Returns the number of spectres currently owned by the level.
    int getSpectreCount() const;

    // Returns the number of guarded item/anchor pairs.
    int getGuardedItemCount() const;

    // Returns the Cipher Gem carrier position, if any.
    std::optional<Position> getCipherCarrierPosition() const;

    // Returns lightweight position/symbol pairs for rendering.
    std::vector<std::pair<Position, char>> getItemRenderObjects() const;

    // Returns lightweight position/symbol pairs for spectre rendering.
    std::vector<std::pair<Position, char>> getSpectreRenderObjects() const;
};

#endif
