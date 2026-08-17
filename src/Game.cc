#include "Game.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Layout files may either describe one reusable level or all five levels.
std::vector<std::string> layoutRowsForLevel(
    const std::vector<std::string> &rows,
    int levelNumber) {
    // layout describes 1 reusable level & returns all rows unchanged
    if (rows.size() == static_cast<std::size_t>(Map::Height)) {
        return rows;
    }
    // 5- level layout , contains 125 rows, 25 rows per level
    if (rows.size() != static_cast<std::size_t>(Map::Height * 5)) {
        throw std::invalid_argument{
            "Layout file must contain either 25 rows or 125 rows"};
    }
    if (levelNumber < 1 || levelNumber > 5) {
        throw std::invalid_argument{"Layout level number must be between 1 and 5"};
    }

    std::vector<std::string> levelRows;
    int startRow = (levelNumber - 1) * Map::Height;
    for (int row = 0; row < Map::Height; ++row) {
        levelRows.emplace_back(rows[startRow + row]);
    }

    return levelRows;
}

} // namespace

std::size_t scrollIndex(ScrollType type) {
    return static_cast<std::size_t>(type);
}

std::string scrollEffectText(ScrollType type) {
    switch (type) {
        case ScrollType::AttuneFocus:
            return "Attune Focus (+10 FP)";
        case ScrollType::SurgePower:
            return "Surge Power (+5 Power)";
        case ScrollType::FortifyWard:
            return "Fortify Ward (+5 Ward)";
        case ScrollType::DrainFocus:
            return "Drain Focus (-10 FP)";
        case ScrollType::SapPower:
            return "Sap Power (-5 Power)";
        case ScrollType::ErodeWard:
            return "Erode Ward (-5 Ward)";
    }

    return "Unknown Scroll";
}

std::string unlockedScrollsText(const std::array<bool, 6> &unlockedScrolls) {
    const std::array<ScrollType, 6> scrollTypes{
        ScrollType::AttuneFocus,
        ScrollType::SurgePower,
        ScrollType::FortifyWard,
        ScrollType::DrainFocus,
        ScrollType::SapPower,
        ScrollType::ErodeWard
    };

    std::string text;
    for (ScrollType type: scrollTypes) {
        if (!unlockedScrolls[scrollIndex(type)]) {
            continue;
        }
        if (!text.empty()) {
            text += ", ";
        }
        text += scrollEffectText(type);
    }

    if (text.empty()) {
        return "None";
    }
    return text;
}

// Constructs a new game using a time-seeded RNG.
Game::Game():
    currentLevel{1},
    display{},
    rng{},
    arcanistFactory{},
    arcanist{nullptr},
    state{nullptr},
    message{},
    layoutRows{std::nullopt},
    aegisCloakLevel{rng.range(2, 4)},
    quitRequested{false},
    enableSpecterLordChase{false},
    unlockedScrolls{} {
    setState(StateKind::ClassSelection);
}

// Constructs a new game using a fixed seed for repeatable tests.
Game::Game(unsigned int seed, bool enableSpecterLordChase):
    currentLevel{1},
    display{},
    rng{seed},
    arcanistFactory{},
    arcanist{nullptr},
    state{nullptr},
    message{},
    layoutRows{std::nullopt},
    aegisCloakLevel{rng.range(2, 4)},
    quitRequested{false},
    enableSpecterLordChase{enableSpecterLordChase},
    unlockedScrolls{} {
    setState(StateKind::ClassSelection);
}

// Constructs a layout-file game using a time-seeded RNG.
Game::Game(const std::vector<std::string> &layoutRows):
    currentLevel{1, Map::createFromLayoutRows(layoutRowsForLevel(layoutRows, 1))},
    display{},
    rng{},
    arcanistFactory{},
    arcanist{nullptr},
    state{nullptr},
    message{},
    layoutRows{layoutRows},
    aegisCloakLevel{rng.range(2, 4)},
    quitRequested{false},
    enableSpecterLordChase{false},
    unlockedScrolls{} {
    setState(StateKind::ClassSelection);
}

// Constructs a layout-file game using a deterministic RNG.
Game::Game(
    const std::vector<std::string> &layoutRows,
    unsigned int seed,
    bool enableSpecterLordChase):
    currentLevel{1, Map::createFromLayoutRows(layoutRowsForLevel(layoutRows, 1))},
    display{},
    rng{seed},
    arcanistFactory{},
    arcanist{nullptr},
    state{nullptr},
    message{},
    layoutRows{layoutRows},
    aegisCloakLevel{rng.range(2, 4)},
    quitRequested{false},
    enableSpecterLordChase{enableSpecterLordChase},
    unlockedScrolls{} {
    setState(StateKind::ClassSelection);
}

// Toggles whether random Specter Lords use the bonus chase movement.
void Game::setSpecterLordChaseEnabled(bool enabled) {
    enableSpecterLordChase = enabled;
}

// Draws the model by passing terrain, item symbols, Arcanist position, and status to the View.
void Game::draw(std::ostream &out) const {
    std::optional<Position> arcanistPosition;

    if (arcanist) {
        arcanistPosition = arcanist->getPosition();
    }

    display.draw(
        out,
        currentLevel,
        makeStatusLine(),
        message,
        arcanistPosition,
        currentLevel.getItemRenderObjects(),
        currentLevel.getSpectreRenderObjects());
}

// Sends one input command to the current State object.
void Game::handleCommand(const std::string &command) {
    if (state) {
        state->handleCommand(*this, command);
    }
}

// Changes the current State object and runs the old/new transition hooks.
void Game::setState(StateKind nextState) {
    if (state) {
        state->onExit(*this);
    }

    // ==================== DESIGN PATTERN: State ====================
    // Game switches between concrete State objects instead of using a large
    // if/else chain for every command in every phase of the game.
    switch (nextState) {
        case StateKind::ClassSelection:
            state = std::make_unique<ClassSelectionState>();
            break;
        case StateKind::Playing:
            state = std::make_unique<PlayingState>();
            break;
        case StateKind::Won:
            state = std::make_unique<WonState>();
            break;
        case StateKind::Lost:
            state = std::make_unique<LostState>();
            break;
    }

    state->onEnter(*this);
}

// Creates the selected Arcanist, generates starting level items, and enters gameplay.
void Game::selectClass(char classCode) {
    // ==================== DESIGN PATTERN: Factory Method / Simple Factory ====================
    // Game asks the factory for an AbstractArcanist. After this point Game uses
    // the abstract interface, not Sage/Hexblade/Warden/Voidwalker directly.
    arcanist = arcanistFactory.create(classCode, randomSpawnPosition());
    setState(StateKind::Playing);
    startCurrentLevel();
}

// Attempts one-square Arcanist movement and collects fragments on the destination tile.
bool Game::moveArcanist(Direction direction) {
    if (!arcanist) {
        setMessage("Choose an Arcanist class before moving.");
        return false;
    }

    Position nextPosition = arcanist->getPosition() + directionOffset(direction);

    if (!currentLevel.getMap().inBounds(nextPosition) ||
        !currentLevel.getMap().getCell(nextPosition).isWalkable()) {
        setMessage("The Arcanist cannot move there.");
        return false;
    }
    if (currentLevel.getSpectreAt(nextPosition)) {
        setMessage("A spectre blocks the way.");
        return false;
    }

    arcanist->setPosition(nextPosition);
    setMessage("The Arcanist moves.");
    collectItemIfPresent();
    if (handleStairwayIfPresent()) {
        return true;
    }
    processSpectreTurns();
    return true;
}

// Attempts to use an adjacent scroll and remove it from the level after use.
bool Game::useScroll(Direction direction) {
    if (!arcanist) {
        setMessage("Choose an Arcanist class before using a scroll.");
        return false;
    }

    Position scrollPosition = arcanist->getPosition() + directionOffset(direction);
    Item *item = currentLevel.getItemAt(scrollPosition);

    if (!item || item->getCategory() != ItemCategory::Scroll) {
        setMessage("There is no scroll there.");
        return false;
    }

    ScrollType type = item->getScrollType();
    std::string scrollName = item->getName();
    unlockedScrolls[scrollIndex(type)] = true;
    applyScroll(type);
    currentLevel.removeItemAt(scrollPosition);
    setMessage("Used scroll: " + scrollName + ".");
    if (updateLossIfArcanistDefeated()) {
        return true;
    }
    processSpectreTurns();
    return true;
}

// Attempts one Arcanist attack against an adjacent spectre.
bool Game::attackSpectre(Direction direction) {
    if (!arcanist) {
        setMessage("Choose an Arcanist class before attacking.");
        return false;
    }

    Position targetPosition = arcanist->getPosition() + directionOffset(direction);
    AbstractSpectre *spectre = currentLevel.getSpectreAt(targetPosition);

    if (!spectre) {
        setMessage("There is no spectre there.");
        return false;
    }

    int damage = calculateDamage(arcanist->getPower(), spectre->getWard());
    std::string spectreName = spectre->getName();
    spectre->takeDamage(damage);
    int remainingFP = spectre->getFocusPoints();

    if (spectre->isDefeated()) {
        bool collectedCipherGem = handleSpectreDefeat(targetPosition, *spectre);
        if (collectedCipherGem) {
            setMessage(
                "Hit " + spectreName + " for " + std::to_string(damage) +
                " damage. " + spectreName + " FP: 0. Defeated " +
                spectreName + " and recovered the Cipher Gem. The stairway is revealed.");
        } else {
            setMessage(
                "Hit " + spectreName + " for " + std::to_string(damage) +
                " damage. " + spectreName + " FP: 0. Defeated " + spectreName + ".");
        }
    } else {
        setMessage(
            "Hit " + spectreName + " for " + std::to_string(damage) +
            " damage. " + spectreName + " FP: " + std::to_string(remainingFP) + ".");
    }

    processSpectreTurns();
    return true;
}

// Moves all spectres using their MovementStrategy objects.
void Game::moveSpectres() {
    if (!arcanist) {
        return;
    }

    currentLevel.moveSpectres(rng, arcanist->getPosition());
}

// Runs the full spectre turn pass after a consumed player command.
void Game::runSpectreTurns() {
    processSpectreTurns();
}

// Clears all temporary stat decorators from the Arcanist.
void Game::resetTemporaryStats() {
    if (arcanist) {
        arcanist->resetTemporaryStats();
    }
}

// Adds an item to the current level, mainly for tests and future layout loading.
void Game::addItem(std::unique_ptr<Item> item) {
    // Ownership is passed through Game into the current VaultLevel.
    currentLevel.addItem(std::move(item));
}

// Adds a spectre to the current level, mainly for tests and future layout loading.
void Game::addSpectre(std::unique_ptr<AbstractSpectre> spectre) {
    // Ownership is passed through Game into the current VaultLevel.
    currentLevel.addSpectre(std::move(spectre));
}

// Resets all current run data back to a fresh class-selection state.
void Game::restart() {
    if (layoutRows) {
        currentLevel = VaultLevel{1, Map::createFromLayoutRows(layoutRowsForLevel(layoutRows.value(), 1))};
    } else {
        currentLevel = VaultLevel{1};
    }
    arcanist.reset();
    aegisCloakLevel = rng.range(2, 4);
    quitRequested = false;
    unlockedScrolls.fill(false);
    setState(StateKind::ClassSelection);
}

// Marks the game loop as finished.
void Game::quit() {
    quitRequested = true;
    message = "Goodbye.";
}

// Reports whether the controller should stop reading commands.
bool Game::shouldQuit() const {
    return quitRequested;
}

// Reports whether class selection has created an Arcanist yet.
bool Game::hasArcanist() const {
    return arcanist != nullptr;
}

// Returns the current State object's display name.
std::string Game::getStateName() const {
    if (!state) {
        return "None";
    }

    return state->getName();
}

// Returns the selected class name, or "--" before class selection.
std::string Game::getSelectedClassName() const {
    if (!arcanist) {
        return "--";
    }

    return arcanist->getClassName();
}

// Returns current Arcanist FP, using 0 as a safe pre-selection value.
int Game::getArcanistFP() const {
    if (!arcanist) {
        return 0;
    }

    return arcanist->getCurrentFP();
}

// Returns current Arcanist Power, using 0 as a safe pre-selection value.
int Game::getArcanistPower() const {
    if (!arcanist) {
        return 0;
    }

    return arcanist->getPower();
}

// Returns current Arcanist Ward Rating, using 0 as a safe pre-selection value.
int Game::getArcanistWard() const {
    if (!arcanist) {
        return 0;
    }

    return arcanist->getWard();
}

// Returns collected Rune Fragment value, using 0 before class selection.
int Game::getRuneFragments() const {
    if (!arcanist) {
        return 0;
    }

    return arcanist->getRuneFragments();
}

// Returns whether the current level's Cipher Gem is carried.
bool Game::hasCipherGem() const {
    return arcanist && arcanist->hasCipherGem();
}

// Returns whether the Aegis Cloak is carried.
bool Game::hasAegisCloak() const {
    return arcanist && arcanist->hasAegisCloak();
}

// Returns final score after class ability bonuses.
int Game::getFinalScore() const {
    if (!arcanist) {
        return 0;
    }

    return arcanist->getFinalScore();
}

// Applies class-specific incoming damage modification.
int Game::modifyIncomingDamage(int damage) const {
    if (!arcanist) {
        return damage;
    }

    return arcanist->modifyIncomingDamage(damage);
}

// Calculates ceil((100 / (100 + WR(defender))) * Pwr(attacker)).
int Game::calculateDamage(int attackerPower, int defenderWard) const {
    return static_cast<int>(
        std::ceil((100.0 / (100.0 + static_cast<double>(defenderWard))) *
                  static_cast<double>(attackerPower)));
}

// Returns the Arcanist position; this is invalid before class selection.
Position Game::getArcanistPosition() const {
    if (!arcanist) {
        throw std::logic_error{"No Arcanist has been selected yet"};
    }

    return arcanist->getPosition();
}

// Returns the current vault level number.
int Game::getLevelNumber() const {
    return currentLevel.getLevelNumber();
}

// Returns the number of level-owned items still active.
int Game::getItemCount() const {
    return currentLevel.getItemCount();
}

// Returns the number of level-owned spectres still active.
int Game::getSpectreCount() const {
    return currentLevel.getSpectreCount();
}

// Returns read-only access to the level for tests and display support.
const VaultLevel &Game::getCurrentLevel() const {
    return currentLevel;
}

// Updates the status message shown by the View.
void Game::setMessage(const std::string &newMessage) {
    message = newMessage;
}

// Adds a same-turn event without hiding earlier messages.
void Game::appendMessage(const std::string &newMessage) {
    if (message.empty()) {
        message = newMessage;
    } else {
        message += " " + newMessage;
    }
}

// Selects the current 25-row layout chunk from a 25-row or 125-row layout file.
std::vector<std::string> Game::currentLayoutRows() const {
    if (!layoutRows) {
        throw std::logic_error{"No layout file is active"};
    }

    return layoutRowsForLevel(layoutRows.value(), currentLevel.getLevelNumber());
}

// Builds the one-line status text printed in the reserved status rows.
std::string Game::makeStatusLine() const {
    if (!arcanist) {
        return "State: " + getStateName() +
               "  Level: " + std::to_string(getLevelNumber()) +
               "  Class: --  FP: --  Pwr: --  WR: --  Fragments: --";
    }

    return "State: " + getStateName() +
           "  Level: " + std::to_string(getLevelNumber()) +
           "  Class: " + arcanist->getClassName() +
           "  FP: " + std::to_string(arcanist->getCurrentFP()) +
           "  Pwr: " + std::to_string(arcanist->getPower()) +
           "  WR: " + std::to_string(arcanist->getWard()) +
           "  Fragments: " + std::to_string(arcanist->getRuneFragments()) +
           "\nUnlocked Scrolls: " + unlockedScrollsText(unlockedScrolls);
}

// Chooses a random floor tile from any chamber for the Arcanist spawn.
Position Game::randomSpawnPosition() {
    std::vector<Position> allTiles;
    std::vector<std::string> activeRows;
    if (layoutRows) {
        activeRows = currentLayoutRows();
    }

    for (int chamberId = 1; chamberId <= 5; ++chamberId) {
        std::vector<Position> chamberTiles = currentLevel.getMap().getSpawnTiles(chamberId);
        for (const Position &pos: chamberTiles) {
            if (layoutRows &&
                std::isdigit(static_cast<unsigned char>(activeRows[pos.row][pos.col]))) {
                continue;
            }
            allTiles.emplace_back(pos);
        }
    }

    if (allTiles.empty()) {
        throw std::logic_error{"No legal Arcanist spawn tiles exist"};
    }

    return allTiles[rng.range(0, static_cast<int>(allTiles.size()) - 1)];
}

// Handles automatic pickup when the Arcanist lands on a rune fragment.
void Game::collectItemIfPresent() {
    if (!arcanist) {
        return;
    }

    Item *item = currentLevel.getItemAt(arcanist->getPosition());
    if (!item) {
        return;
    }
    if (currentLevel.isGuardedItemLocked(item->getPosition())) {
        appendMessage("The item is still guarded by a Vault Anchor.");
        return;
    }

    if (item->getCategory() == ItemCategory::MajorItem) {
        collectMajorItem(*item);
        currentLevel.removeItemAt(arcanist->getPosition());
        return;
    }
    if (item->getCategory() != ItemCategory::RuneFragment) {
        return;
    }

    int value = item->getFragmentValue();
    std::string itemName = item->getName();
    arcanist->addRuneFragments(value);
    currentLevel.removeItemAt(arcanist->getPosition());
    appendMessage("Collected " + itemName + " worth " + std::to_string(value) + ".");
}

// Applies major item pickup effects.
void Game::collectMajorItem(Item &item) {
    MajorItemType type = item.getMajorItemType();

    if (type == MajorItemType::CipherGem) {
        arcanist->collectCipherGem();
        currentLevel.revealStairway();
        appendMessage("Collected the Cipher Gem. The stairway is revealed.");
    } else if (type == MajorItemType::AegisCloak) {
        arcanist->collectAegisCloak();
        appendMessage("Collected the Aegis Cloak.");
    }
}

// Applies the correct reward/drop for a defeated spectre and removes it from the level.
bool Game::handleSpectreDefeat(const Position &position, const AbstractSpectre &spectre) {
    if (!arcanist) {
        return false;
    }

    SpectreType type = spectre.getType();
    bool hadCipherGem = spectre.hasCipherGem();
    currentLevel.removeSpectreAt(position);

    if (hadCipherGem) {
        // CIPHER GEM AUTO-PICKUP:
        // Defeating the carrier gives the gem directly to the Arcanist instead
        // of dropping a C item that the player must step away from and re-enter.
        arcanist->collectCipherGem();
        currentLevel.revealStairway();
        return true;
    } else if (type == SpectreType::Lich) {
        ItemFactory factory;
        currentLevel.addItem(factory.createFragment(FragmentType::LichCache, position));
    } else if (type != SpectreType::VaultAnchor) {
        arcanist->addRuneFragments(1);
    }

    return false;
}

// Resolves spectre turns and applies attacks returned by hostile spectres.
void Game::processSpectreTurns() {
    if (!arcanist || arcanist->getCurrentFP() == 0) {
        return;
    }

    std::vector<AbstractSpectre *> attackers =
        currentLevel.takeSpectreTurns(rng, arcanist->getPosition());

    for (const AbstractSpectre *spectre: attackers) {
        if (arcanist->getCurrentFP() == 0) {
            return;
        }
        resolveSpectreAttack(*spectre);
    }
}

// Resolves one hostile spectre attack with the spec's 50% miss chance.
void Game::resolveSpectreAttack(const AbstractSpectre &spectre) {
    if (!arcanist) {
        return;
    }

    if (rng.chance(1, 2)) {
        appendMessage(spectre.getName() + " missed.");
        return;
    }

    int damage = calculateDamage(spectre.getPower(), arcanist->getWard());
    damage = arcanist->modifyIncomingDamage(damage);
    if (arcanist->hasAegisCloak()) {
        damage = std::max(1, (damage + 1) / 2);
    }
    arcanist->changeFP(-damage);
    appendMessage(spectre.getName() + " hit for " + std::to_string(damage) + " damage.");

    if (arcanist->getCurrentFP() == 0) {
        setState(StateKind::Lost);
    }
}

// Applies scroll effects; temporary Power/Ward effects use the Decorator pattern.
void Game::applyScroll(ScrollType type) {
    if (!arcanist) {
        return;
    }

    // ==================== DESIGN PATTERN: Strategy ====================
    // Class abilities can transform or multiply scroll effects without Game
    // checking concrete classes like Hexblade or Voidwalker.
    ScrollType transformedType = arcanist->transformScroll(type, rng);
    int multiplier = arcanist->getScrollEffectMultiplier();

    for (int i = 0; i < multiplier; ++i) {
        applySingleScrollEffect(transformedType);
    }
}

// Applies one scroll effect once; applyScroll handles class ability rules first.
void Game::applySingleScrollEffect(ScrollType type) {
    if (!arcanist) {
        return;
    }

    if (type == ScrollType::AttuneFocus) {
        arcanist->changeFP(10);
    } else if (type == ScrollType::SurgePower) {
        arcanist->addSurgePowerEffect();
    } else if (type == ScrollType::FortifyWard) {
        arcanist->addFortifyWardEffect();
    } else if (type == ScrollType::DrainFocus) {
        arcanist->changeFP(-10);
    } else if (type == ScrollType::SapPower) {
        arcanist->addSapPowerEffect();
    } else if (type == ScrollType::ErodeWard) {
        arcanist->addErodeWardEffect();
    }
}

// Generates all level-owned objects for the current vault level.
void Game::startCurrentLevel() {
    if (!arcanist) {
        return;
    }

    // ==================== DESIGN PATTERN: Factory Method / Simple Factory ====================
    // VaultLevel delegates concrete item/spectre creation to factories while
    // Game only coordinates when a level starts.
    if (layoutRows) {
        currentLevel.loadItemsFromLayoutRows(rng, currentLayoutRows(), arcanist->getPosition());
    } else {
        currentLevel.generateBasicItems(
            rng,
            arcanist->getPosition(),
            currentLevel.getLevelNumber() == aegisCloakLevel);
    }
    currentLevel.generateSpectres(rng, arcanist->getPosition(), enableSpecterLordChase);

    // EXTENSION: Ritualist
    // Ritualists begin every level with one random scroll effect already active.
    // The class-specific choice is exposed through ArcanistAbility, so this
    // level-start hook works without checking whether the concrete class is Ritualist.
    if (arcanist->startsLevelWithRandomScrollEffect()) {
        ScrollType randomScrollType = static_cast<ScrollType>(rng.range(0, 5));
        applyScroll(randomScrollType);
        appendMessage("Ritualist started this level with " + scrollEffectText(randomScrollType) + ".");
        updateLossIfArcanistDefeated();
    }
}

// Descends when the Arcanist is standing on a revealed stairway.
bool Game::handleStairwayIfPresent() {
    if (!arcanist ||
        !currentLevel.isStairwayVisible() ||
        arcanist->getPosition() != currentLevel.getHiddenStairway()) {
        return false;
    }

    if (currentLevel.getLevelNumber() == 5) {
        setState(StateKind::Won);
        return true;
    }

    advanceToNextLevel();
    return true;
}

// Keeps the same Arcanist object but replaces the level-owned objects.
void Game::advanceToNextLevel() {
    if (!arcanist) {
        return;
    }

    int nextLevelNumber = currentLevel.getLevelNumber() + 1;

    // Temporary Decorator effects and the Cipher Gem are level-specific.
    arcanist->resetTemporaryStats();
    arcanist->resetCipherGem();

    if (layoutRows) {
        currentLevel = VaultLevel{
            nextLevelNumber,
            Map::createFromLayoutRows(layoutRowsForLevel(layoutRows.value(), nextLevelNumber))};
    } else {
        currentLevel = VaultLevel{nextLevelNumber};
    }
    arcanist->setPosition(randomSpawnPosition());
    startCurrentLevel();
    if (arcanist->getCurrentFP() == 0) {
        return;
    }
    appendMessage("Descended to vault level " + std::to_string(nextLevelNumber) + ".");
}

// Switches to Lost state whenever FP has reached 0.
bool Game::updateLossIfArcanistDefeated() {
    if (arcanist && arcanist->getCurrentFP() == 0) {
        setState(StateKind::Lost);
        return true;
    }

    return false;
}
