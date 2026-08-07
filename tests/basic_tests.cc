#include <cassert>
#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include "ArcanistFactory.h"
#include "Cell.h"
#include "Game.h"
#include "Direction.h"
#include "ItemFactory.h"
#include "Map.h"
#include "Position.h"
#include "Random.h"
#include "SpectreFactory.h"
#include "Stats.h"
#include "TextDisplay.h"
#include "VaultLevel.h"

void testPositionAndDirection() {
    Position start{10, 10};
    Direction direction = Direction::North;

    assert(parseDirection("ne", direction));
    Position northEast{9, 11};
    assert(start + directionOffset(direction) == northEast);

    assert(parseDirection("sw", direction));
    Position southWest{11, 9};
    assert(start + directionOffset(direction) == southWest);

    assert(!parseDirection("bad", direction));
    assert(!parseDirection("", direction));
    assert(!parseDirection("NO", direction));
    assert(!parseDirection("ne ", direction));
}

void testCellSymbols() {
    assert(Cell{CellType::Empty}.getSymbol() == ' ');
    assert(Cell{CellType::WallVertical}.getSymbol() == '|');
    assert(Cell{CellType::WallHorizontal}.getSymbol() == '-');
    Cell floor{CellType::Floor, 1};
    assert(floor.getSymbol() == '.');
    assert(Cell{CellType::Archway}.getSymbol() == '+');
    assert(Cell{CellType::Corridor}.getSymbol() == '#');
}

void testDefaultMapDimensions() {
    Map map = Map::createDefault();
    std::vector<std::string> rows = map.toRows();

    assert(static_cast<int>(rows.size()) == Map::Height);
    for (const std::string &row: rows) {
        assert(static_cast<int>(row.length()) == Map::Width);
    }
}

void testDefaultMapTerrain() {
    Map map = Map::createDefault();

    assert(map.getCell(Position{3, 5}).getType() == CellType::Floor);
    assert(map.getCell(Position{3, 5}).getChamberId() == 1);
    assert(map.getCell(Position{4, 15}).getType() == CellType::Corridor);
    assert(map.getCell(Position{4, 14}).getType() == CellType::Archway);
    assert(map.getCell(Position{4, 22}).getType() == CellType::Corridor);
    assert(map.getCell(Position{4, 23}).getType() == CellType::Archway);
    assert(map.getCell(Position{7, 9}).getType() == CellType::Corridor);
    assert(map.getCell(Position{8, 9}).getType() == CellType::Corridor);
    assert(map.getCell(Position{0, 0}).getType() == CellType::Empty);
}

void testLayoutMapParsingConvertsDigitsToFloor() {
    std::vector<std::string> rows = Map::createDefault().toRows();
    rows[3][5] = '0';
    rows[3][6] = '9';

    Map map = Map::createFromLayoutRows(rows);

    assert(map.getCell(Position{3, 5}).getType() == CellType::Floor);
    assert(map.getCell(Position{3, 6}).getType() == CellType::Floor);
    assert(map.getCell(Position{3, 5}).getChamberId() == map.getCell(Position{3, 6}).getChamberId());
    assert(map.toRows()[3][5] == '.');
}

void testLayoutMapParsingRejectsBadInput() {
    std::vector<std::string> rows = Map::createDefault().toRows();

    bool shortLayoutThrew = false;
    try {
        std::vector<std::string> tooFewRows = rows;
        tooFewRows.pop_back();
        Map::createFromLayoutRows(tooFewRows);
    } catch (const std::invalid_argument &) {
        shortLayoutThrew = true;
    }
    assert(shortLayoutThrew);

    bool wideRowThrew = false;
    try {
        std::vector<std::string> wideRows = rows;
        wideRows[0] += '.';
        Map::createFromLayoutRows(wideRows);
    } catch (const std::invalid_argument &) {
        wideRowThrew = true;
    }
    assert(wideRowThrew);

    bool badSymbolThrew = false;
    try {
        std::vector<std::string> badRows = rows;
        badRows[3][5] = '@';
        Map::createFromLayoutRows(badRows);
    } catch (const std::invalid_argument &) {
        badSymbolThrew = true;
    }
    assert(badSymbolThrew);
}

void testMapBoundsEdgeCases() {
    Map map = Map::createDefault();

    assert(map.inBounds(Position{0, 0}));
    assert(map.inBounds(Position{Map::Height - 1, Map::Width - 1}));
    assert(!map.inBounds(Position{-1, 0}));
    assert(!map.inBounds(Position{0, -1}));
    assert(!map.inBounds(Position{Map::Height, 0}));
    assert(!map.inBounds(Position{0, Map::Width}));

    bool threw = false;
    try {
        map.getCell(Position{-1, 0});
    } catch (const std::out_of_range &) {
        threw = true;
    }
    assert(threw);
}

void testSpawnTiles() {
    Map map = Map::createDefault();

    for (int chamberId = 1; chamberId <= 5; ++chamberId) {
        std::vector<Position> tiles = map.getSpawnTiles(chamberId);
        assert(!tiles.empty());

        for (const Position &pos: tiles) {
            const Cell &cell = map.getCell(pos);
            assert(cell.isSpawnFloor());
            assert(cell.getChamberId() == chamberId);
        }
    }

    assert(map.getSpawnTiles(0).empty());
    assert(map.getSpawnTiles(6).empty());
}

void testTextDisplayRows() {
    VaultLevel level{1};
    TextDisplay display;
    std::ostringstream out;

    display.draw(out, level, "status line", "test message");

    int rowCount = 0;
    std::string line;
    std::istringstream in{out.str()};
    while (std::getline(in, line)) {
        ++rowCount;
    }

    assert(rowCount == 30);
}

void testTextDisplayCommandHelpIncludesImplementedActions() {
    VaultLevel level{1};
    TextDisplay display;
    std::ostringstream out;

    display.draw(out, level, "status line", "test message");

    std::string rendered = out.str();
    assert(rendered.find("u dir use") != std::string::npos);
    assert(rendered.find("a dir attack") != std::string::npos);
    assert(rendered.find("r restart") != std::string::npos);
}

void testTextDisplayUsesAnsiColours() {
    VaultLevel level{1};
    TextDisplay display;
    std::ostringstream out;
    std::optional<Position> arcanist = Position{3, 5};
    std::vector<std::pair<Position, char>> items{
        {Position{3, 6}, '?'},
        {Position{3, 7}, '*'}
    };

    display.draw(out, level, "status line", "message", arcanist, items);

    std::string rendered = out.str();
    assert(rendered.find("\033[") != std::string::npos);
    assert(rendered.find("@\033[0m") != std::string::npos);
    assert(rendered.find("?\033[0m") != std::string::npos);
    assert(rendered.find("*\033[0m") != std::string::npos);
}

bool findAdjacentFreeSpawnFloor(
    const Game &game,
    Direction &chosenDirection,
    Position &chosenPosition) {
    std::vector<Direction> directions{
        Direction::North,
        Direction::South,
        Direction::East,
        Direction::West,
        Direction::NorthEast,
        Direction::NorthWest,
        Direction::SouthEast,
        Direction::SouthWest
    };

    Position start = game.getArcanistPosition();

    for (Direction direction: directions) {
        Position candidate = start + directionOffset(direction);
        const VaultLevel &level = game.getCurrentLevel();
        if (level.getMap().inBounds(candidate) &&
            level.getMap().getCell(candidate).isSpawnFloor() &&
            !level.getItemAt(candidate) &&
            !level.getSpectreAt(candidate)) {
            chosenDirection = direction;
            chosenPosition = candidate;
            return true;
        }
    }

    return false;
}

bool directionBetweenAdjacentPositions(
    const Position &start,
    const Position &target,
    Direction &chosenDirection) {
    std::vector<Direction> directions{
        Direction::North,
        Direction::South,
        Direction::East,
        Direction::West,
        Direction::NorthEast,
        Direction::NorthWest,
        Direction::SouthEast,
        Direction::SouthWest
    };

    for (Direction direction: directions) {
        if (start + directionOffset(direction) == target) {
            chosenDirection = direction;
            return true;
        }
    }

    return false;
}

void testInitialGameState() {
    Game game;

    assert(game.getStateName() == "Class Selection");
    assert(game.getSelectedClassName() == "--");
    assert(!game.hasArcanist());
    assert(!game.shouldQuit());
    assert(game.getLevelNumber() == 1);
}

void testClassSelectionCommands() {
    Game game;

    game.handleCommand("no");
    assert(game.getStateName() == "Class Selection");
    assert(!game.hasArcanist());

    game.handleCommand("s");
    assert(game.getStateName() == "Playing");
    assert(game.getSelectedClassName() == "Sage");
    assert(game.hasArcanist());
    assert(game.getArcanistFP() == 120);
    assert(game.getArcanistPower() == 18);
    assert(game.getArcanistWard() == 18);

    game.handleCommand("r");
    assert(game.getStateName() == "Class Selection");
    assert(game.getSelectedClassName() == "--");
    assert(!game.hasArcanist());

    game.handleCommand("v");
    assert(game.getStateName() == "Playing");
    assert(game.getSelectedClassName() == "Voidwalker");
    assert(game.getArcanistFP() == 110);
    assert(game.getArcanistPower() == 22);
    assert(game.getArcanistWard() == 12);
}

void testEndStatesBlockGameplayAndAllowRestartQuit() {
    Game wonGame{50};
    wonGame.handleCommand("s");
    Position wonPosition = wonGame.getArcanistPosition();
    wonGame.setState(StateKind::Won);
    wonGame.handleCommand("no");
    wonGame.handleCommand("a no");
    wonGame.handleCommand("u no");
    assert(wonGame.getStateName() == "Won");
    assert(wonGame.getArcanistPosition() == wonPosition);
    wonGame.handleCommand("r");
    assert(wonGame.getStateName() == "Class Selection");
    assert(!wonGame.hasArcanist());

    Game lostGame{51};
    lostGame.handleCommand("h");
    Position lostPosition = lostGame.getArcanistPosition();
    lostGame.setState(StateKind::Lost);
    lostGame.handleCommand("ea");
    assert(lostGame.getStateName() == "Lost");
    assert(lostGame.getArcanistPosition() == lostPosition);
    lostGame.handleCommand("q");
    assert(lostGame.shouldQuit());
}

void testInvalidClassAndQuit() {
    Game game;

    game.handleCommand("x");
    assert(game.getStateName() == "Class Selection");
    assert(game.getSelectedClassName() == "--");
    assert(!game.shouldQuit());

    game.handleCommand("q");
    assert(game.shouldQuit());
}

void testArcanistFactoryStats() {
    ArcanistFactory factory;
    Position pos{3, 5};

    std::unique_ptr<AbstractArcanist> sage = factory.create('s', pos);
    assert(sage->getClassName() == "Sage");
    assert(sage->getCurrentFP() == 120);
    assert(sage->getMaxFP() == 120);
    assert(sage->getPower() == 18);
    assert(sage->getWard() == 18);
    assert(sage->getRuneFragments() == 0);
    assert(sage->getPosition() == pos);
    assert(sage->getSymbol() == '@');

    std::unique_ptr<AbstractArcanist> hexblade = factory.create('h', pos);
    assert(hexblade->getClassName() == "Hexblade");
    assert(hexblade->getCurrentFP() == 100);
    assert(hexblade->getPower() == 24);
    assert(hexblade->getWard() == 14);

    std::unique_ptr<AbstractArcanist> warden = factory.create('w', pos);
    assert(warden->getClassName() == "Warden");
    assert(warden->getCurrentFP() == 150);
    assert(warden->getPower() == 16);
    assert(warden->getWard() == 22);

    std::unique_ptr<AbstractArcanist> voidwalker = factory.create('v', pos);
    assert(voidwalker->getClassName() == "Voidwalker");
    assert(voidwalker->getCurrentFP() == 110);
    assert(voidwalker->getPower() == 22);
    assert(voidwalker->getWard() == 12);
}

void testArcanistAbilityStrategiesDirectly() {
    ArcanistFactory factory;
    Position pos{3, 5};

    std::unique_ptr<AbstractArcanist> sage = factory.create('s', pos);
    sage->addRuneFragments(7);
    assert(sage->getFinalScore() == 10);

    std::unique_ptr<AbstractArcanist> hexblade = factory.create('h', pos);
    assert(hexblade->getScrollEffectMultiplier() == 2);

    std::unique_ptr<AbstractArcanist> warden = factory.create('w', pos);
    assert(warden->modifyIncomingDamage(10) == 9);
    assert(warden->modifyIncomingDamage(1) == 1);

    std::unique_ptr<AbstractArcanist> voidwalker = factory.create('v', pos);
    bool sawConversion = false;
    bool sawNoConversion = false;

    for (unsigned int seed = 0; seed < 100; ++seed) {
        Random rng{seed};
        ScrollType transformed = voidwalker->transformScroll(ScrollType::DrainFocus, rng);
        if (transformed == ScrollType::AttuneFocus) {
            sawConversion = true;
        } else if (transformed == ScrollType::DrainFocus) {
            sawNoConversion = true;
        }
    }

    assert(sawConversion);
    assert(sawNoConversion);
}

void testArcanistFactoryInvalidClass() {
    ArcanistFactory factory;
    Position pos{3, 5};
    bool threw = false;

    try {
        factory.create('x', pos);
    } catch (const std::invalid_argument &) {
        threw = true;
    }

    assert(threw);
}

void testGameArcanistSpawnIsLegal() {
    Game game;
    game.handleCommand("h");

    Position pos = game.getArcanistPosition();
    Map map = Map::createDefault();
    const Cell &cell = map.getCell(pos);

    assert(cell.isSpawnFloor());
}

void testGeneratedItemsAreLegalAndNonOverlapping() {
    Game game{123};
    game.handleCommand("s");
    const VaultLevel &level = game.getCurrentLevel();

    assert(level.getItemCount() == 20);
    assert(level.getGuardedItemCount() == 1);
    assert(!level.getItemAt(game.getArcanistPosition()));
    assert(!level.getItemAt(level.getHiddenStairway()));
    assert(level.getMap().getCell(level.getHiddenStairway()).isSpawnFloor());

    std::vector<Position> seen;
    for (const auto &object: level.getItemRenderObjects()) {
        assert(level.getMap().getCell(object.first).isSpawnFloor());
        assert(object.first != game.getArcanistPosition());
        assert(object.first != level.getHiddenStairway());
        assert(std::find(seen.begin(), seen.end(), object.first) == seen.end());
        seen.emplace_back(object.first);
    }
}

void testLayoutDigitItemsLoadCorrectTypes() {
    std::vector<std::string> rows = Map::createDefault().toRows();
    for (int digit = 0; digit <= 9; ++digit) {
        rows[3][5 + digit] = static_cast<char>('0' + digit);
    }

    Random rng{7};
    VaultLevel level{1, Map::createFromLayoutRows(rows)};
    level.loadItemsFromLayoutRows(rng, rows, Position{3, 24});

    assert(level.getItemCount() == 10);
    assert(level.getItemAt(Position{3, 5})->getScrollType() == ScrollType::AttuneFocus);
    assert(level.getItemAt(Position{3, 6})->getScrollType() == ScrollType::SurgePower);
    assert(level.getItemAt(Position{3, 7})->getScrollType() == ScrollType::FortifyWard);
    assert(level.getItemAt(Position{3, 8})->getScrollType() == ScrollType::DrainFocus);
    assert(level.getItemAt(Position{3, 9})->getScrollType() == ScrollType::SapPower);
    assert(level.getItemAt(Position{3, 10})->getScrollType() == ScrollType::ErodeWard);
    assert(level.getItemAt(Position{3, 11})->getFragmentType() == FragmentType::CommonShard);
    assert(level.getItemAt(Position{3, 12})->getFragmentType() == FragmentType::ResonantShard);
    assert(level.getItemAt(Position{3, 13})->getFragmentType() == FragmentType::LichCache);
    assert(level.getItemAt(Position{3, 14})->getFragmentType() == FragmentType::ShardCoffer);
}

void testLayoutGameModeUsesSeedAndSkipsRandomItems() {
    std::vector<std::string> rows = Map::createDefault().toRows();
    rows[3][5] = '0';
    rows[3][6] = '6';

    Game first{rows, 99};
    Game second{rows, 99};
    first.handleCommand("s");
    second.handleCommand("s");

    assert(first.getArcanistPosition() == second.getArcanistPosition());
    assert(first.getCurrentLevel().getHiddenStairway() == second.getCurrentLevel().getHiddenStairway());
    Position firstDigit{3, 5};
    Position secondDigit{3, 6};
    assert(first.getArcanistPosition() != firstDigit);
    assert(first.getArcanistPosition() != secondDigit);
    assert(first.getItemCount() == 2);
    assert(first.getSpectreCount() == 20);
    assert(first.getCurrentLevel().getCipherCarrierPosition());
}

void testFiveLevelLayoutRowsAreAccepted() {
    std::vector<std::string> baseRows = Map::createDefault().toRows();
    std::vector<std::string> allRows;

    // A 125-row layout represents five 25-row levels in one file.
    for (int level = 0; level < 5; ++level) {
        std::vector<std::string> levelRows = baseRows;
        levelRows[3][5 + level] = '6';
        allRows.insert(allRows.end(), levelRows.begin(), levelRows.end());
    }

    Game game{allRows, 40};
    game.handleCommand("s");

    assert(game.getLevelNumber() == 1);
    assert(game.getItemCount() == 1);
    assert(game.getSpectreCount() == 20);
}

void testGeneratedCipherCarrierExistsAndIsNotAnchor() {
    Game game{321};
    game.handleCommand("s");

    std::optional<Position> carrierPosition = game.getCurrentLevel().getCipherCarrierPosition();
    assert(carrierPosition);

    const AbstractSpectre *carrier = game.getCurrentLevel().getSpectreAt(carrierPosition.value());
    assert(carrier);
    assert(carrier->hasCipherGem());
    assert(carrier->getType() != SpectreType::VaultAnchor);
}

void testGuardedItemLockAndUnlock() {
    VaultLevel level{1};
    ItemFactory itemFactory;
    Random rng{33};

    Position itemPosition{3, 5};
    Position anchorPosition{3, 6};
    level.addGuardedItem(
        itemFactory.createFragment(FragmentType::ShardCoffer, itemPosition),
        anchorPosition);

    assert(level.getGuardedItemCount() == 1);
    assert(level.getSpectreAt(anchorPosition));
    assert(level.isGuardedItemLocked(itemPosition));
    assert(level.isArcanistAdjacentToGuardedItem(anchorPosition, Position{4, 5}));
    assert(!level.isArcanistAdjacentToGuardedItem(anchorPosition, Position{10, 10}));

    std::vector<AbstractSpectre *> attackers =
        level.takeSpectreTurns(rng, Position{4, 5});
    assert(attackers.size() == 1);
    assert(attackers[0]->getType() == SpectreType::VaultAnchor);

    level.removeSpectreAt(anchorPosition);
    assert(!level.isGuardedItemLocked(itemPosition));
}

void testSpectreFactoryCreatesAllTypes() {
    SpectreFactory factory;
    Position pos{3, 5};
    int chamberId = 1;

    std::unique_ptr<AbstractSpectre> wraith =
        factory.create(SpectreType::Wraith, pos, chamberId);
    assert(wraith->getName() == "Wraith");
    assert(wraith->getSymbol() == 'W');
    assert(wraith->getFocusPoints() == 60);
    assert(wraith->getPower() == 20);
    assert(wraith->getWard() == 10);
    assert(wraith->getSpawnChamberId() == chamberId);
    assert(wraith->getMovementStrategyName() == "Random Movement");
    assert(wraith->getHostilityStrategyName() == "Always Hostile");

    std::unique_ptr<AbstractSpectre> banshee =
        factory.create(SpectreType::Banshee, pos, chamberId);
    assert(banshee->getName() == "Banshee");
    assert(banshee->getSymbol() == 'B');
    assert(banshee->getFocusPoints() == 40);
    assert(banshee->getPower() == 30);
    assert(banshee->getWard() == 5);

    std::unique_ptr<AbstractSpectre> revenant =
        factory.create(SpectreType::Revenant, pos, chamberId);
    assert(revenant->getSymbol() == 'R');
    assert(revenant->getFocusPoints() == 100);
    assert(revenant->getPower() == 18);
    assert(revenant->getWard() == 20);

    std::unique_ptr<AbstractSpectre> shade =
        factory.create(SpectreType::Shade, pos, chamberId);
    assert(shade->getSymbol() == 'S');
    assert(shade->getFocusPoints() == 50);
    assert(shade->getPower() == 8);
    assert(shade->getWard() == 8);

    std::unique_ptr<AbstractSpectre> lich =
        factory.create(SpectreType::Lich, pos, chamberId);
    assert(lich->getSymbol() == 'L');
    assert(lich->getFocusPoints() == 30);
    assert(lich->getPower() == 40);
    assert(lich->getWard() == 5);

    std::unique_ptr<AbstractSpectre> anchor =
        factory.create(SpectreType::VaultAnchor, pos, chamberId);
    assert(anchor->getSymbol() == 'A');
    assert(anchor->getFocusPoints() == 120);
    assert(anchor->getPower() == 22);
    assert(anchor->getWard() == 18);
    assert(anchor->getMovementStrategyName() == "Stationary Movement");
    assert(anchor->getHostilityStrategyName() == "Guarded Item Hostility");

    std::unique_ptr<AbstractSpectre> lord =
        factory.create(SpectreType::SpecterLord, pos, chamberId);
    assert(lord->getSymbol() == 'X');
    assert(lord->getFocusPoints() == 80);
    assert(lord->getPower() == 28);
    assert(lord->getWard() == 15);
    assert(lord->getMovementStrategyName() == "Chase Movement");
    assert(lord->getHostilityStrategyName() == "Always Hostile");
}

void testSpectreMovementLegalityChecks() {
    VaultLevel level{1};
    SpectreFactory spectreFactory;
    ItemFactory itemFactory;
    Position arcanist{3, 10};
    Position spectrePos{3, 5};
    Position itemPos{3, 6};
    Position otherSpectrePos{4, 5};

    level.addItem(itemFactory.createFragment(FragmentType::CommonShard, itemPos));
    level.addSpectre(spectreFactory.create(SpectreType::Wraith, spectrePos, 1));
    level.addSpectre(spectreFactory.create(SpectreType::Shade, otherSpectrePos, 1));

    const AbstractSpectre *spectre = level.getSpectreAt(spectrePos);
    assert(spectre);

    assert(level.canSpectreMoveTo(*spectre, Position{3, 7}, arcanist));
    assert(!level.canSpectreMoveTo(*spectre, itemPos, arcanist));
    assert(!level.canSpectreMoveTo(*spectre, otherSpectrePos, arcanist));
    assert(!level.canSpectreMoveTo(*spectre, arcanist, arcanist));
    assert(!level.canSpectreMoveTo(*spectre, Position{2, 5}, arcanist));
    assert(!level.canSpectreMoveTo(*spectre, Position{14, 5}, arcanist));
}

void testStationaryMovementDoesNotMoveVaultAnchor() {
    VaultLevel level{1};
    SpectreFactory factory;
    Random rng{30};
    Position anchorPos{3, 5};
    Position arcanist{3, 10};

    level.addSpectre(factory.create(SpectreType::VaultAnchor, anchorPos, 1));
    level.moveSpectres(rng, arcanist);

    assert(level.getSpectreAt(anchorPos));
}

void testRandomMovementStaysInSpawnChamber() {
    VaultLevel level{1};
    SpectreFactory factory;
    Random rng{31};
    Position start{3, 5};
    Position arcanist{3, 10};

    level.addSpectre(factory.create(SpectreType::Wraith, start, 1));
    level.moveSpectres(rng, arcanist);

    std::vector<std::pair<Position, char>> renderObjects = level.getSpectreRenderObjects();
    assert(renderObjects.size() == 1);

    Position after = renderObjects[0].first;
    assert(after != arcanist);
    assert(level.getMap().getCell(after).isSpawnFloor());
    assert(level.getMap().getCell(after).getChamberId() == 1);
    assert(after.row >= start.row - 1 && after.row <= start.row + 1);
    assert(after.col >= start.col - 1 && after.col <= start.col + 1);
}

void testChaseMovementMovesTowardArcanist() {
    VaultLevel level{1};
    SpectreFactory factory;
    Random rng{40};
    Position start{4, 24};
    Position arcanist{4, 13};

    level.addSpectre(factory.create(SpectreType::SpecterLord, start, 2));
    level.moveSpectres(rng, arcanist);

    assert(!level.getSpectreAt(start));
    assert(level.getSpectreAt(Position{4, 23}));
}

void testChaseMovementRoutesThroughCorridors() {
    VaultLevel level{1};
    SpectreFactory factory;
    Random rng{41};
    Position start{4, 24};
    Position arcanist{4, 13};

    level.addSpectre(factory.create(SpectreType::SpecterLord, start, 2));

    for (int i = 0; i < 5; ++i) {
        level.moveSpectres(rng, arcanist);
    }

    assert(level.getSpectreAt(Position{4, 19}));
    assert(level.getMap().getCell(Position{4, 19}).getType() == CellType::Corridor);
}

void testChaseMovementRespectsBlockedPath() {
    VaultLevel level{1};
    SpectreFactory spectreFactory;
    ItemFactory itemFactory;
    Random rng{42};
    Position start{3, 5};
    Position arcanist{3, 9};

    level.addItem(itemFactory.createFragment(FragmentType::CommonShard, Position{3, 6}));
    level.addItem(itemFactory.createFragment(FragmentType::CommonShard, Position{4, 5}));
    level.addItem(itemFactory.createFragment(FragmentType::CommonShard, Position{4, 6}));
    level.addSpectre(spectreFactory.create(SpectreType::SpecterLord, start, 1));

    level.moveSpectres(rng, arcanist);

    assert(level.getSpectreAt(start));
}

void testBlockedSpectreHasNoLegalMove() {
    VaultLevel level{1};
    SpectreFactory spectreFactory;
    ItemFactory itemFactory;
    Random rng{32};
    Position center{3, 5};
    Position arcanist{3, 10};

    level.addItem(itemFactory.createFragment(FragmentType::CommonShard, Position{3, 6}));
    level.addItem(itemFactory.createFragment(FragmentType::CommonShard, Position{4, 5}));
    level.addItem(itemFactory.createFragment(FragmentType::CommonShard, Position{4, 6}));
    level.addSpectre(spectreFactory.create(SpectreType::Wraith, center, 1));

    level.moveSpectres(rng, arcanist);
    assert(level.getSpectreAt(center));
}

void testGameMoveSpectresKeepsCountAndAvoidsArcanist() {
    Game game{33};
    game.handleCommand("s");
    Position arcanist = game.getArcanistPosition();

    game.moveSpectres();

    assert(game.getSpectreCount() == 21);
    assert(!game.getCurrentLevel().getSpectreAt(arcanist));

    for (const auto &object: game.getCurrentLevel().getSpectreRenderObjects()) {
        const AbstractSpectre *spectre = game.getCurrentLevel().getSpectreAt(object.first);
        assert(spectre);
        if (spectre->getMovementStrategyName() == "Chase Movement") {
            assert(game.getCurrentLevel().getMap().getCell(object.first).isWalkable());
        } else {
            assert(game.getCurrentLevel().getMap().getCell(object.first).getChamberId() ==
                   spectre->getSpawnChamberId());
        }
    }
}

void testHostileSpectreAttacksInsteadOfMoving() {
    VaultLevel level{1};
    SpectreFactory factory;
    Random rng{34};
    Position arcanist{3, 6};
    Position spectrePosition{3, 5};

    level.addSpectre(factory.create(SpectreType::Wraith, spectrePosition, 1));
    std::vector<AbstractSpectre *> attackers = level.takeSpectreTurns(rng, arcanist);

    assert(attackers.size() == 1);
    assert(attackers[0]->getPosition() == spectrePosition);
    assert(level.getSpectreAt(spectrePosition));
}

void testNeutralLichMovesInsteadOfAttacking() {
    VaultLevel level{1};
    SpectreFactory factory;
    Random rng{35};
    Position arcanist{3, 6};
    Position spectrePosition{3, 5};

    level.addSpectre(factory.create(SpectreType::Lich, spectrePosition, 1));
    std::vector<AbstractSpectre *> attackers = level.takeSpectreTurns(rng, arcanist);

    assert(attackers.empty());
    assert(!level.getSpectreAt(spectrePosition) || level.getSpectreAt(spectrePosition)->getType() == SpectreType::Lich);
}

void testInvalidPlayerActionDoesNotTriggerSpectreTurn() {
    Game game{36};
    SpectreFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));
    game.addSpectre(factory.create(SpectreType::Wraith, spectrePosition, 1));

    int fpBefore = game.getArcanistFP();
    game.handleCommand("u bad");
    assert(game.getArcanistFP() == fpBefore);
}

void testValidPlayerActionTriggersSpectreAttack() {
    Game game{37};
    SpectreFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));
    game.addSpectre(factory.create(SpectreType::Banshee, spectrePosition, 1));

    for (int i = 0; i < 20 && game.getArcanistFP() == 120; ++i) {
        game.runSpectreTurns();
    }

    assert(game.getArcanistFP() < 120);
}

void testWardenReducesSpectreAttackDamage() {
    Game game{38};
    SpectreFactory factory;
    game.handleCommand("w");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));
    game.addSpectre(factory.create(SpectreType::Banshee, spectrePosition, 1));

    int before = game.getArcanistFP();
    for (int i = 0; i < 20 && game.getArcanistFP() == before; ++i) {
        game.runSpectreTurns();
    }

    assert(before - game.getArcanistFP() == 24);
}

void testWardenAndAegisDamageOrder() {
    Game game{40};
    game.handleCommand("w");

    int rawDamage = game.calculateDamage(30, game.getArcanistWard());
    int afterWarden = game.modifyIncomingDamage(rawDamage);
    int afterAegis = std::max(1, (afterWarden + 1) / 2);

    assert(rawDamage == 25);
    assert(afterWarden == 24);
    assert(afterAegis == 12);
}

void testSpectreAttackCanCauseLoss() {
    Game game{39};
    SpectreFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));
    game.addSpectre(factory.create(SpectreType::Banshee, spectrePosition, 1));

    for (int i = 0; i < 200 && game.getStateName() != "Lost"; ++i) {
        game.runSpectreTurns();
    }

    assert(game.getStateName() == "Lost");
    assert(game.getArcanistFP() == 0);
}

void testGeneratedSpectresAreLegalAndNonOverlapping() {
    Game game{123};
    game.handleCommand("s");
    const VaultLevel &level = game.getCurrentLevel();

    assert(game.getSpectreCount() == 21);

    std::vector<Position> seen;
    for (const auto &object: level.getSpectreRenderObjects()) {
        assert(level.getMap().getCell(object.first).isSpawnFloor());
        assert(object.first != game.getArcanistPosition());
        assert(object.first != level.getHiddenStairway());
        assert(!level.getItemAt(object.first));
        assert(std::find(seen.begin(), seen.end(), object.first) == seen.end());
        seen.emplace_back(object.first);

        const AbstractSpectre *spectre = level.getSpectreAt(object.first);
        assert(spectre);
        assert(spectre->getSpawnChamberId() == level.getMap().getCell(object.first).getChamberId());
    }
}

void testVaultLevelSpectreEdgeCases() {
    VaultLevel level{1};
    SpectreFactory factory;
    ItemFactory itemFactory;
    Position floor{3, 5};
    Position wall{2, 4};

    bool nullThrew = false;
    try {
        level.addSpectre(nullptr);
    } catch (const std::invalid_argument &) {
        nullThrew = true;
    }
    assert(nullThrew);

    bool wallThrew = false;
    try {
        level.addSpectre(factory.create(SpectreType::Wraith, wall, 1));
    } catch (const std::invalid_argument &) {
        wallThrew = true;
    }
    assert(wallThrew);

    level.addItem(itemFactory.createFragment(FragmentType::CommonShard, floor));
    bool itemOverlapThrew = false;
    try {
        level.addSpectre(factory.create(SpectreType::Wraith, floor, 1));
    } catch (const std::invalid_argument &) {
        itemOverlapThrew = true;
    }
    assert(itemOverlapThrew);

    Position floor2{3, 6};
    level.addSpectre(factory.create(SpectreType::Shade, floor2, 1));
    bool duplicateThrew = false;
    try {
        level.addSpectre(factory.create(SpectreType::Lich, floor2, 1));
    } catch (const std::invalid_argument &) {
        duplicateThrew = true;
    }
    assert(duplicateThrew);

    bool itemOnSpectreThrew = false;
    try {
        level.addItem(itemFactory.createScroll(ScrollType::AttuneFocus, floor2));
    } catch (const std::invalid_argument &) {
        itemOnSpectreThrew = true;
    }
    assert(itemOnSpectreThrew);
}

void testVaultLevelItemEdgeCases() {
    VaultLevel level{1};
    ItemFactory factory;
    Position floor{3, 5};
    Position wall{2, 4};

    assert(level.removeItemAt(floor) == nullptr);

    bool nullThrew = false;
    try {
        level.addItem(nullptr);
    } catch (const std::invalid_argument &) {
        nullThrew = true;
    }
    assert(nullThrew);

    bool wallThrew = false;
    try {
        level.addItem(factory.createScroll(ScrollType::AttuneFocus, wall));
    } catch (const std::invalid_argument &) {
        wallThrew = true;
    }
    assert(wallThrew);

    level.addItem(factory.createFragment(FragmentType::CommonShard, floor));
    assert(level.getItemCount() == 1);

    bool duplicateThrew = false;
    try {
        level.addItem(factory.createScroll(ScrollType::DrainFocus, floor));
    } catch (const std::invalid_argument &) {
        duplicateThrew = true;
    }
    assert(duplicateThrew);

    std::unique_ptr<Item> removed = level.removeItemAt(floor);
    assert(removed != nullptr);
    assert(level.getItemCount() == 0);
}

void testItemFactoryTypes() {
    ItemFactory factory;
    Position pos{3, 5};

    std::unique_ptr<Item> scroll = factory.createScroll(ScrollType::DrainFocus, pos);
    assert(scroll->getCategory() == ItemCategory::Scroll);
    assert(scroll->getSymbol() == '?');
    assert(scroll->getScrollType() == ScrollType::DrainFocus);

    std::unique_ptr<Item> fragment = factory.createFragment(FragmentType::ResonantShard, pos);
    assert(fragment->getCategory() == ItemCategory::RuneFragment);
    assert(fragment->getSymbol() == '*');
    assert(fragment->getFragmentValue() == 2);

    std::unique_ptr<Item> cloak = factory.createMajorItem(MajorItemType::AegisCloak, pos);
    assert(cloak->getCategory() == ItemCategory::MajorItem);
    assert(cloak->getSymbol() == 'G');
}

void testFragmentPickup() {
    Game game{5};
    ItemFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    int beforeCount = game.getItemCount();
    game.addItem(factory.createFragment(FragmentType::ResonantShard, itemPosition));
    assert(game.getItemCount() == beforeCount + 1);

    assert(game.moveArcanist(direction));
    assert(game.getRuneFragments() == 2);
    assert(game.getItemCount() == beforeCount);
    assert(!game.getCurrentLevel().getItemAt(itemPosition));
}

void testScrollUseAndEdgeCases() {
    Game game{9};
    ItemFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    int beforeCount = game.getItemCount();
    game.addItem(factory.createScroll(ScrollType::DrainFocus, itemPosition));
    assert(game.useScroll(direction));
    assert(game.getArcanistFP() <= 110);
    assert(game.getItemCount() == beforeCount);
    assert(!game.getCurrentLevel().getItemAt(itemPosition));

    assert(!game.useScroll(direction));

    int fpBeforeInvalidCommand = game.getArcanistFP();
    int countBeforeInvalidCommand = game.getItemCount();
    game.handleCommand("u");
    game.handleCommand("u bad");
    game.handleCommand("u no extra");
    assert(game.getArcanistFP() == fpBeforeInvalidCommand);
    assert(game.getItemCount() == countBeforeInvalidCommand);
}

void testHexbladeDoublesScrollEffectsInGame() {
    Game game{18};
    ItemFactory factory;
    game.handleCommand("h");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    game.addItem(factory.createScroll(ScrollType::DrainFocus, itemPosition));
    assert(game.useScroll(direction));
    assert(game.getArcanistFP() == 80);

    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));
    game.addItem(factory.createScroll(ScrollType::SurgePower, itemPosition));
    assert(game.useScroll(direction));
    assert(game.getArcanistPower() == 34);
}

void testIncomingDamageStrategyInGame() {
    Game sage{19};
    sage.handleCommand("s");
    assert(sage.modifyIncomingDamage(10) == 10);

    Game warden{20};
    warden.handleCommand("w");
    assert(warden.modifyIncomingDamage(10) == 9);
    assert(warden.modifyIncomingDamage(1) == 1);

    Game noArcanist{21};
    assert(noArcanist.modifyIncomingDamage(10) == 10);
}

void testDamageFormula() {
    Game game{23};

    assert(game.calculateDamage(18, 5) == 18);
    assert(game.calculateDamage(20, 10) == 19);
    assert(game.calculateDamage(40, 5) == 39);
    assert(game.calculateDamage(0, 10) == 0);
}

void testSageFinalScoreInGame() {
    Game game{22};
    ItemFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    game.addItem(factory.createFragment(FragmentType::ResonantShard, itemPosition));
    assert(game.moveArcanist(direction));
    assert(game.getRuneFragments() == 2);
    assert(game.getFinalScore() == 3);
}

void testAttackBeforeClassSelectionFails() {
    Game game{24};

    assert(!game.attackSpectre(Direction::North));
    assert(!game.hasArcanist());
}

void testAttackInvalidTargetAndMalformedCommand() {
    Game game{25};
    game.handleCommand("s");

    Position before = game.getArcanistPosition();
    int fragmentsBefore = game.getRuneFragments();

    game.handleCommand("a");
    game.handleCommand("a bad");
    game.handleCommand("a no extra");
    assert(game.getArcanistPosition() == before);
    assert(game.getRuneFragments() == fragmentsBefore);

    Direction direction = Direction::North;
    Position freePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, freePosition));
    assert(!game.attackSpectre(direction));
}

void testAttackDamagesSpectreWithoutDefeatingIt() {
    Game game{26};
    SpectreFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));

    game.addSpectre(factory.create(SpectreType::Banshee, spectrePosition, 1));
    assert(game.attackSpectre(direction));

    const AbstractSpectre *spectre = game.getCurrentLevel().getSpectreAt(spectrePosition);
    assert(spectre);
    assert(spectre->getFocusPoints() == 22);
    assert(game.getRuneFragments() == 0);
}

void testDefeatingRegularSpectreAwardsFragment() {
    Game game{27};
    SpectreFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));

    int beforeSpectres = game.getSpectreCount();
    game.addSpectre(factory.create(SpectreType::Banshee, spectrePosition, 1));

    assert(game.attackSpectre(direction));
    assert(game.attackSpectre(direction));
    assert(game.attackSpectre(direction));

    assert(!game.getCurrentLevel().getSpectreAt(spectrePosition));
    assert(game.getSpectreCount() == beforeSpectres);
    assert(game.getRuneFragments() == 1);
}

void testDefeatingLichDropsCache() {
    Game game{28};
    SpectreFactory factory;
    ItemFactory itemFactory;
    game.handleCommand("h");

    Direction scrollDirection = Direction::North;
    Position scrollPosition;
    assert(findAdjacentFreeSpawnFloor(game, scrollDirection, scrollPosition));
    game.addItem(itemFactory.createScroll(ScrollType::SurgePower, scrollPosition));
    assert(game.useScroll(scrollDirection));
    assert(game.getStateName() == "Playing");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));

    int beforeSpectres = game.getSpectreCount();
    int chamberId = game.getCurrentLevel().getMap().getCell(spectrePosition).getChamberId();
    game.addSpectre(factory.create(SpectreType::Lich, spectrePosition, chamberId));
    int beforeItems = game.getItemCount();

    assert(game.attackSpectre(direction));

    assert(!game.getCurrentLevel().getSpectreAt(spectrePosition));
    const Item *drop = game.getCurrentLevel().getItemAt(spectrePosition);
    assert(drop);
    assert(drop->getCategory() == ItemCategory::RuneFragment);
    assert(drop->getFragmentType() == FragmentType::LichCache);
    assert(drop->getFragmentValue() == 4);
    assert(game.getItemCount() == beforeItems + 1);
    assert(game.getSpectreCount() == beforeSpectres);
    assert(game.getRuneFragments() == 0);
}

void testCipherGemDropPickupAndReveal() {
    Game game{30};
    SpectreFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));

    std::unique_ptr<AbstractSpectre> carrier =
        factory.create(SpectreType::Banshee, spectrePosition, 1);
    carrier->setCipherGemCarrier(true);
    game.addSpectre(std::move(carrier));

    for (int attacks = 0;
         attacks < 10 && game.getCurrentLevel().getSpectreAt(spectrePosition);
         ++attacks) {
        assert(game.attackSpectre(direction));
    }

    assert(!game.getCurrentLevel().getSpectreAt(spectrePosition));
    const Item *drop = game.getCurrentLevel().getItemAt(spectrePosition);
    assert(drop);
    assert(drop->getCategory() == ItemCategory::MajorItem);
    assert(drop->getMajorItemType() == MajorItemType::CipherGem);
    assert(!game.hasCipherGem());
    assert(!game.getCurrentLevel().isStairwayVisible());

    assert(game.moveArcanist(direction));
    assert(game.hasCipherGem());
    assert(game.getCurrentLevel().isStairwayVisible());
    assert(!game.getCurrentLevel().getItemAt(spectrePosition));
}

void testMovementBlockedByManualSpectre() {
    Game game{29};
    SpectreFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position spectrePosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, spectrePosition));
    Position before = game.getArcanistPosition();

    game.addSpectre(factory.create(SpectreType::Shade, spectrePosition, 1));
    assert(!game.moveArcanist(direction));
    assert(game.getArcanistPosition() == before);
}

void testUseScrollBeforeClassSelectionFails() {
    Game game{10};

    assert(!game.useScroll(Direction::North));
    assert(!game.hasArcanist());
}

void testMovingOntoScrollDoesNotUseIt() {
    Game game{14};
    ItemFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    int beforeCount = game.getItemCount();
    game.addItem(factory.createScroll(ScrollType::DrainFocus, itemPosition));
    assert(game.moveArcanist(direction));

    assert(game.getArcanistFP() == 120);
    assert(game.getItemCount() == beforeCount + 1);
    assert(game.getCurrentLevel().getItemAt(itemPosition));
}

void testMovingOntoMajorItemCollectsAegis() {
    Game game{15};
    ItemFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    int beforeCount = game.getItemCount();
    game.addItem(factory.createMajorItem(MajorItemType::AegisCloak, itemPosition));
    assert(game.moveArcanist(direction));

    assert(game.getItemCount() == beforeCount);
    assert(!game.getCurrentLevel().getItemAt(itemPosition));
    assert(game.hasAegisCloak());
}

void testRevealedStairwayAdvancesLevelAndResetsTemporaryState() {
    ItemFactory factory;
    bool foundUsableSeed = false;

    for (unsigned int seed = 0; seed < 5000 && !foundUsableSeed; ++seed) {
        Game game{seed};
        game.handleCommand("s");

        Direction scrollDirection = Direction::North;
        Position scrollPosition;
        if (!findAdjacentFreeSpawnFloor(game, scrollDirection, scrollPosition)) {
            continue;
        }

        game.addItem(factory.createScroll(ScrollType::SurgePower, scrollPosition));
        if (!game.useScroll(scrollDirection) || game.getStateName() != "Playing") {
            continue;
        }
        assert(game.getArcanistPower() == 23);

        Position stairPosition = game.getCurrentLevel().getHiddenStairway();
        Direction stairDirection = Direction::North;
        if (!directionBetweenAdjacentPositions(
                game.getArcanistPosition(),
                stairPosition,
                stairDirection)) {
            continue;
        }
        if (game.getCurrentLevel().getItemAt(stairPosition) ||
            game.getCurrentLevel().getSpectreAt(stairPosition)) {
            continue;
        }

        game.addItem(factory.createMajorItem(MajorItemType::CipherGem, stairPosition));
        assert(game.moveArcanist(stairDirection));

        assert(game.getStateName() == "Playing");
        assert(game.getLevelNumber() == 2);
        assert(game.getArcanistPower() == 18);
        assert(!game.hasCipherGem());
        assert(!game.getCurrentLevel().isStairwayVisible());
        assert(game.getItemCount() >= 20);
        assert(game.getSpectreCount() >= 21);

        foundUsableSeed = true;
    }

    assert(foundUsableSeed);
}

void testDrainFocusCanCauseLossBeforeSpectreTurns() {
    Game game{42};
    ItemFactory factory;
    game.handleCommand("h");

    for (int i = 0; i < 5 && game.getStateName() == "Playing"; ++i) {
        Direction direction = Direction::North;
        Position itemPosition;
        assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

        game.addItem(factory.createScroll(ScrollType::DrainFocus, itemPosition));
        assert(game.useScroll(direction));
    }

    assert(game.getArcanistFP() == 0);
    assert(game.getStateName() == "Lost");
}

void testInvalidItemPlacementTerrain() {
    VaultLevel level{1};
    ItemFactory factory;

    std::vector<Position> invalidPositions{
        Position{-1, 0},
        Position{4, 15},
        Position{4, 14}
    };

    for (const Position &pos: invalidPositions) {
        bool threw = false;
        try {
            level.addItem(factory.createFragment(FragmentType::CommonShard, pos));
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        assert(threw);
    }
}

void testGetArcanistPositionBeforeClassThrows() {
    Game game;
    bool threw = false;

    try {
        game.getArcanistPosition();
    } catch (const std::logic_error &) {
        threw = true;
    }

    assert(threw);
}

void testRestartClearsArcanistAndItems() {
    Game game{16};
    game.handleCommand("s");
    assert(game.hasArcanist());
    assert(game.getItemCount() == 20);
    assert(game.getSpectreCount() == 21);

    game.handleCommand("r");
    assert(!game.hasArcanist());
    assert(game.getItemCount() == 0);
    assert(game.getSpectreCount() == 0);
    assert(game.getStateName() == "Class Selection");
}

void testTextDisplayIgnoresOutOfBoundsObjects() {
    VaultLevel level{1};
    TextDisplay display;
    std::ostringstream out;

    std::optional<Position> badArcanist = Position{-1, -1};
    std::vector<std::pair<Position, char>> badObjects{
        {Position{-1, 0}, '*'},
        {Position{Map::Height, Map::Width}, '?'}
    };

    display.draw(out, level, "status line", "message", badArcanist, badObjects);

    int rowCount = 0;
    std::string line;
    std::istringstream in{out.str()};
    while (std::getline(in, line)) {
        ++rowCount;
    }

    assert(rowCount == 30);
}

void testWhitespaceCommandParsing() {
    Game game{17};
    ItemFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    game.addItem(factory.createScroll(ScrollType::SurgePower, itemPosition));
    game.handleCommand("u    no");
    assert(game.getArcanistPower() == 23);

    int powerBefore = game.getArcanistPower();
    game.handleCommand("   ");
    assert(game.getArcanistPower() == powerBefore);
}

void testScrollStatLimits() {
    ArcanistFactory arcanistFactory;
    Position pos{3, 5};
    std::unique_ptr<AbstractArcanist> arcanist = arcanistFactory.create('s', pos);

    arcanist->changeFP(10);
    assert(arcanist->getCurrentFP() == arcanist->getMaxFP());

    arcanist->changeFP(-1000);
    assert(arcanist->getCurrentFP() == 0);

    arcanist->changePower(-1000);
    assert(arcanist->getPower() == 0);

    arcanist->changeWard(-1000);
    assert(arcanist->getWard() == 0);
}

void testStatsDecoratorStackingAndReset() {
    std::unique_ptr<Stats> stats = std::make_unique<BaseStats>(18, 18);

    stats = std::make_unique<SurgePowerStats>(std::move(stats));
    assert(stats->getPower() == 23);
    assert(stats->getWard() == 18);

    stats = std::make_unique<FortifyWardStats>(std::move(stats));
    assert(stats->getPower() == 23);
    assert(stats->getWard() == 23);

    stats = std::make_unique<SapPowerStats>(std::move(stats));
    assert(stats->getPower() == 18);
    assert(stats->getWard() == 23);

    stats = std::make_unique<ErodeWardStats>(std::move(stats));
    assert(stats->getPower() == 18);
    assert(stats->getWard() == 18);
}

void testStatsDecoratorFloorsAtZero() {
    std::unique_ptr<Stats> stats = std::make_unique<BaseStats>(3, 4);

    stats = std::make_unique<SapPowerStats>(std::move(stats));
    stats = std::make_unique<SapPowerStats>(std::move(stats));
    stats = std::make_unique<ErodeWardStats>(std::move(stats));
    stats = std::make_unique<ErodeWardStats>(std::move(stats));

    assert(stats->getPower() == 0);
    assert(stats->getWard() == 0);
}

void testStatsDecoratorRejectsNullWrappedStats() {
    bool threw = false;

    try {
        std::unique_ptr<Stats> stats;
        SurgePowerStats decorator{std::move(stats)};
    } catch (const std::invalid_argument &) {
        threw = true;
    }

    assert(threw);
}

void testArcanistTemporaryStatsReset() {
    ArcanistFactory factory;
    Position pos{3, 5};
    std::unique_ptr<AbstractArcanist> arcanist = factory.create('s', pos);

    arcanist->addSurgePowerEffect();
    arcanist->addFortifyWardEffect();
    arcanist->addSurgePowerEffect();
    assert(arcanist->getPower() == 28);
    assert(arcanist->getWard() == 23);

    arcanist->resetTemporaryStats();
    assert(arcanist->getPower() == 18);
    assert(arcanist->getWard() == 18);

    arcanist->resetTemporaryStats();
    assert(arcanist->getPower() == 18);
    assert(arcanist->getWard() == 18);
}

void testPermanentStatChangeClearsTemporaryDecorators() {
    ArcanistFactory factory;
    Position pos{3, 5};
    std::unique_ptr<AbstractArcanist> arcanist = factory.create('s', pos);

    arcanist->addSurgePowerEffect();
    assert(arcanist->getPower() == 23);

    arcanist->changePower(5);
    assert(arcanist->getPower() == 23);

    arcanist->resetTemporaryStats();
    assert(arcanist->getPower() == 23);
}

void testMultiplePositiveScrollDecoratorsStack() {
    Game game{12};
    ItemFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    game.addItem(factory.createScroll(ScrollType::SurgePower, itemPosition));
    assert(game.useScroll(direction));
    assert(game.getArcanistPower() == 23);

    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));
    game.addItem(factory.createScroll(ScrollType::SurgePower, itemPosition));
    assert(game.useScroll(direction));
    assert(game.getArcanistPower() == 28);
}

void testFPScrollsSurviveTemporaryStatReset() {
    ArcanistFactory factory;
    Position pos{3, 5};
    std::unique_ptr<AbstractArcanist> arcanist = factory.create('s', pos);

    arcanist->changeFP(-10);
    arcanist->addSurgePowerEffect();
    assert(arcanist->getCurrentFP() == 110);
    assert(arcanist->getPower() == 23);

    arcanist->resetTemporaryStats();
    assert(arcanist->getCurrentFP() == 110);
    assert(arcanist->getPower() == 18);
}

void testGameScrollDecoratorsAndReset() {
    Game game{11};
    ItemFactory factory;
    game.handleCommand("s");

    Direction direction = Direction::North;
    Position itemPosition;
    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));

    int beforeCount = game.getItemCount();
    game.addItem(factory.createScroll(ScrollType::SurgePower, itemPosition));
    assert(game.useScroll(direction));
    assert(game.getArcanistPower() == 23);
    assert(game.getArcanistWard() == 18);
    assert(game.getItemCount() == beforeCount);

    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));
    game.addItem(factory.createScroll(ScrollType::FortifyWard, itemPosition));
    assert(game.useScroll(direction));
    assert(game.getArcanistPower() == 23);
    assert(game.getArcanistWard() == 23);

    assert(findAdjacentFreeSpawnFloor(game, direction, itemPosition));
    game.addItem(factory.createScroll(ScrollType::SapPower, itemPosition));
    assert(game.useScroll(direction));
    assert(game.getArcanistPower() == 18);
    assert(game.getArcanistWard() == 23);

    game.resetTemporaryStats();
    assert(game.getArcanistPower() == 18);
    assert(game.getArcanistWard() == 18);
}

void testMoveBeforeClassSelectionFails() {
    Game game{123};

    assert(!game.moveArcanist(Direction::North));
    assert(!game.hasArcanist());
    assert(game.getStateName() == "Class Selection");
}

void testValidMovementUpdatesPosition() {
    Game game{123};
    game.handleCommand("s");

    Position start = game.getArcanistPosition();
    const VaultLevel &level = game.getCurrentLevel();
    std::vector<Direction> directions{
        Direction::North,
        Direction::South,
        Direction::East,
        Direction::West,
        Direction::NorthEast,
        Direction::NorthWest,
        Direction::SouthEast,
        Direction::SouthWest
    };

    bool moved = false;
    for (Direction direction: directions) {
        Position candidate = start + directionOffset(direction);
        if (level.getMap().inBounds(candidate) &&
            level.getMap().getCell(candidate).isWalkable() &&
            !level.getSpectreAt(candidate)) {
            assert(game.moveArcanist(direction));
            assert(game.getArcanistPosition() == candidate);
            moved = true;
            break;
        }
    }

    assert(moved);
}

void testBlockedMovementDoesNotMove() {
    Game game{456};
    game.handleCommand("h");

    // Moving north repeatedly must eventually hit a wall or empty space.
    // The failed move should leave the Arcanist on the last legal cell.
    bool hitBlockedCell = false;

    for (int steps = 0; steps < Map::Height + Map::Width; ++steps) {
        Position before = game.getArcanistPosition();
        bool moved = game.moveArcanist(Direction::North);
        if (!moved) {
            assert(game.getArcanistPosition() == before);
            hitBlockedCell = true;
            break;
        }
    }

    assert(hitBlockedCell);
}

void testMovementCommandInPlayingState() {
    Game game{789};
    game.handleCommand("w");

    Position before = game.getArcanistPosition();
    const VaultLevel &level = game.getCurrentLevel();
    Position east = before + directionOffset(Direction::East);

    if (level.getMap().inBounds(east) &&
        level.getMap().getCell(east).isWalkable() &&
        !level.getSpectreAt(east)) {
        game.handleCommand("ea");
        assert(game.getArcanistPosition() == east);
    } else {
        game.handleCommand("ea");
        assert(game.getArcanistPosition() == before);
    }
}

void testRandomEdgeCases() {
    Random rng{123};

    assert(rng.range(5, 5) == 5);

    bool rangeThrew = false;
    try {
        rng.range(5, 1);
    } catch (const std::invalid_argument &) {
        rangeThrew = true;
    }
    assert(rangeThrew);

    bool chanceThrew = false;
    try {
        rng.chance(2, 1);
    } catch (const std::invalid_argument &) {
        chanceThrew = true;
    }
    assert(chanceThrew);
}

int main() {
    testPositionAndDirection();
    testCellSymbols();
    testDefaultMapDimensions();
    testDefaultMapTerrain();
    testLayoutMapParsingConvertsDigitsToFloor();
    testLayoutMapParsingRejectsBadInput();
    testMapBoundsEdgeCases();
    testSpawnTiles();
    testTextDisplayRows();
    testTextDisplayCommandHelpIncludesImplementedActions();
    testTextDisplayUsesAnsiColours();
    testInitialGameState();
    testClassSelectionCommands();
    testEndStatesBlockGameplayAndAllowRestartQuit();
    testInvalidClassAndQuit();
    testArcanistFactoryStats();
    testArcanistAbilityStrategiesDirectly();
    testArcanistFactoryInvalidClass();
    testGameArcanistSpawnIsLegal();
    testGeneratedItemsAreLegalAndNonOverlapping();
    testLayoutDigitItemsLoadCorrectTypes();
    testLayoutGameModeUsesSeedAndSkipsRandomItems();
    testFiveLevelLayoutRowsAreAccepted();
    testGeneratedCipherCarrierExistsAndIsNotAnchor();
    testGuardedItemLockAndUnlock();
    testSpectreFactoryCreatesAllTypes();
    testSpectreMovementLegalityChecks();
    testStationaryMovementDoesNotMoveVaultAnchor();
    testRandomMovementStaysInSpawnChamber();
    testChaseMovementMovesTowardArcanist();
    testChaseMovementRoutesThroughCorridors();
    testChaseMovementRespectsBlockedPath();
    testBlockedSpectreHasNoLegalMove();
    testGameMoveSpectresKeepsCountAndAvoidsArcanist();
    testHostileSpectreAttacksInsteadOfMoving();
    testNeutralLichMovesInsteadOfAttacking();
    testInvalidPlayerActionDoesNotTriggerSpectreTurn();
    testValidPlayerActionTriggersSpectreAttack();
    testWardenReducesSpectreAttackDamage();
    testWardenAndAegisDamageOrder();
    testSpectreAttackCanCauseLoss();
    testGeneratedSpectresAreLegalAndNonOverlapping();
    testVaultLevelSpectreEdgeCases();
    testVaultLevelItemEdgeCases();
    testItemFactoryTypes();
    testFragmentPickup();
    testScrollUseAndEdgeCases();
    testHexbladeDoublesScrollEffectsInGame();
    testIncomingDamageStrategyInGame();
    testDamageFormula();
    testSageFinalScoreInGame();
    testAttackBeforeClassSelectionFails();
    testAttackInvalidTargetAndMalformedCommand();
    testAttackDamagesSpectreWithoutDefeatingIt();
    testDefeatingRegularSpectreAwardsFragment();
    testDefeatingLichDropsCache();
    testCipherGemDropPickupAndReveal();
    testMovementBlockedByManualSpectre();
    testUseScrollBeforeClassSelectionFails();
    testMovingOntoScrollDoesNotUseIt();
    testMovingOntoMajorItemCollectsAegis();
    testRevealedStairwayAdvancesLevelAndResetsTemporaryState();
    testDrainFocusCanCauseLossBeforeSpectreTurns();
    testInvalidItemPlacementTerrain();
    testGetArcanistPositionBeforeClassThrows();
    testRestartClearsArcanistAndItems();
    testTextDisplayIgnoresOutOfBoundsObjects();
    testWhitespaceCommandParsing();
    testScrollStatLimits();
    testStatsDecoratorStackingAndReset();
    testStatsDecoratorFloorsAtZero();
    testStatsDecoratorRejectsNullWrappedStats();
    testArcanistTemporaryStatsReset();
    testPermanentStatChangeClearsTemporaryDecorators();
    testMultiplePositiveScrollDecoratorsStack();
    testFPScrollsSurviveTemporaryStatReset();
    testGameScrollDecoratorsAndReset();
    testMoveBeforeClassSelectionFails();
    testValidMovementUpdatesPosition();
    testBlockedMovementDoesNotMove();
    testMovementCommandInPlayingState();
    testRandomEdgeCases();

    return 0;
}
