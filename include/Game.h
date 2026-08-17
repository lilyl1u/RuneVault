#ifndef GAME_H
#define GAME_H

#include <array>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Arcanist.h"
#include "ArcanistFactory.h"
#include "Direction.h"
#include "GameState.h"
#include "Item.h"
#include "Random.h"
#include "TextDisplay.h"
#include "VaultLevel.h"

enum class StateKind {
    ClassSelection,
    Playing,
    Won,
    Lost
};

// ==================== DESIGN PATTERN: MVC - Model ====================
// Game owns the main model state for the current run.
// It exposes simple operations that Controller/State objects can call.
class Game {
    VaultLevel currentLevel;
    TextDisplay display;
    Random rng;
    ArcanistFactory arcanistFactory;
    std::unique_ptr<AbstractArcanist> arcanist;
    std::unique_ptr<GameState> state;
    std::string message;
    std::optional<std::vector<std::string>> layoutRows;
    int aegisCloakLevel;
    bool quitRequested;
    bool enableSpecterLordChase;
    bool lichesProvoked;
    std::array<bool, 6> unlockedScrolls;

    // Builds the current one-line status summary for TextDisplay.
    std::string makeStatusLine() const;

    // Returns the 25 layout rows that belong to the current vault level.
    std::vector<std::string> currentLayoutRows() const;

    // Chooses a legal chamber floor tile for the Arcanist.
    Position randomSpawnPosition();

    // Collects a rune fragment if the Arcanist is standing on one.
    void collectItemIfPresent();

    // Collects a major item if the Arcanist is standing on one.
    void collectMajorItem(Item &item);

    // Handles rewards/drops after a spectre reaches 0 FP; true means Cipher Gem was dropped.
    bool handleSpectreDefeat(const Position &position, const AbstractSpectre &spectre);

    // Runs spectre movement/attack turns after a valid Arcanist action.
    void processSpectreTurns();

    // Applies one spectre attack, including miss chance.
    void resolveSpectreAttack(const AbstractSpectre &spectre);

    // Applies a scroll, including class ability Strategy hooks.
    void applyScroll(ScrollType type);

    // Applies one already-transformed scroll effect once.
    void applySingleScrollEffect(ScrollType type);

    // Generates items/spectres for the current level around the Arcanist.
    void startCurrentLevel();

    // Handles stepping on a revealed stairway after movement.
    bool handleStairwayIfPresent();

    // Moves the existing Arcanist into the next vault level.
    void advanceToNextLevel();

    // Checks whether FP reached 0 after a player-caused effect.
    bool updateLossIfArcanistDefeated();

  public:
    // Creates a game with a time-seeded RNG.
    Game();

    // Creates a game with deterministic RNG, useful for tests.
    Game(unsigned int seed, bool enableSpecterLordChase = false);

    // Creates a layout-file game with a time-seeded RNG.
    explicit Game(const std::vector<std::string> &layoutRows);

    // Creates a layout-file game with deterministic RNG.
    Game(const std::vector<std::string> &layoutRows, unsigned int seed, bool enableSpecterLordChase = false);

    // Enables or disables the optional Specter Lord chase movement.
    void setSpecterLordChaseEnabled(bool enabled);

    // Renders the current model state through TextDisplay.
    void draw(std::ostream &out) const;

    // Delegates one command to the current GameState.
    void handleCommand(const std::string &command);

    // Replaces the current State object and calls enter/exit hooks.
    void setState(StateKind nextState);

    // Creates the selected Arcanist class and enters gameplay.
    void selectClass(char classCode);

    // Attempts to move the Arcanist one square.
    bool moveArcanist(Direction direction);

    // Attempts to use a scroll adjacent to the Arcanist.
    bool useScroll(Direction direction);

    // Attempts to attack an adjacent spectre.
    bool attackSpectre(Direction direction);

    // Moves spectres according to their movement strategies.
    void moveSpectres();

    // Runs spectre turns after a consumed command.
    void runSpectreTurns();

    // Clears temporary stat decorators on the Arcanist.
    void resetTemporaryStats();

    // Adds an item to the current level; useful for generation/tests/layout loading.
    void addItem(std::unique_ptr<Item> item);

    // Adds a spectre to the current level; useful for tests and future layout loading.
    void addSpectre(std::unique_ptr<AbstractSpectre> spectre);

    // Resets the current run back to class selection.
    void restart();

    // Ends the controller loop.
    void quit();

    // Returns whether the player has requested quit.
    bool shouldQuit() const;

    // Returns whether an Arcanist has been selected.
    bool hasArcanist() const;

    // Returns the current state's display name.
    std::string getStateName() const;

    // Returns the selected class name or "--" before selection.
    std::string getSelectedClassName() const;

    // Returns current FP, or 0 before class selection.
    int getArcanistFP() const;

    // Returns current Power, or 0 before class selection.
    int getArcanistPower() const;

    // Returns current Ward Rating, or 0 before class selection.
    int getArcanistWard() const;

    // Returns collected Rune Fragment value, or 0 before class selection.
    int getRuneFragments() const;

    // Returns whether the Arcanist carries the current level's Cipher Gem.
    bool hasCipherGem() const;

    // Returns whether the Arcanist carries the Aegis Cloak.
    bool hasAegisCloak() const;

    // Returns final score after class ability bonuses.
    int getFinalScore() const;

    // Applies the Arcanist class's incoming damage modifier.
    int modifyIncomingDamage(int damage) const;

    // Calculates combat damage using the project formula.
    int calculateDamage(int attackerPower, int defenderWard) const;

    // Returns the Arcanist position; throws if no class has been selected.
    Position getArcanistPosition() const;

    // Returns the current vault level number.
    int getLevelNumber() const;

    // Returns how many items are currently owned by the level.
    int getItemCount() const;

    // Returns how many spectres are currently owned by the level.
    int getSpectreCount() const;

    // Exposes read-only level data for tests and display support.
    const VaultLevel &getCurrentLevel() const;

    // Updates the message shown in the status area.
    void setMessage(const std::string &newMessage);

    // Adds another event to the current status message.
    void appendMessage(const std::string &newMessage);
};

#endif
