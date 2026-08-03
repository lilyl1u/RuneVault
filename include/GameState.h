#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <string>

class Game;

// ==================== DESIGN PATTERN: State ====================
// GameState is the State interface from the State pattern.
// Each concrete state decides which commands make sense during that phase.
class GameState {
  public:
    virtual ~GameState() = default;

    // Returns a short name used in the status row.
    virtual std::string getName() const = 0;

    // Runs when Game changes into this state.
    virtual void onEnter(Game &game);

    // Runs when Game changes away from this state.
    virtual void onExit(Game &game);

    // Handles one command according to this state's rules.
    virtual void handleCommand(Game &game, const std::string &command) = 0;
};

// Concrete State: the player must choose Sage/Hexblade/Warden/Voidwalker.
class ClassSelectionState: public GameState {
  public:
    // Returns the class selection state name.
    std::string getName() const override;

    // Prompts the user to choose an Arcanist class.
    void onEnter(Game &game) override;

    // Accepts class-selection, restart, and quit commands.
    void handleCommand(Game &game, const std::string &command) override;
};

// Concrete State: normal gameplay commands are accepted here.
class PlayingState: public GameState {
  public:
    // Returns the playing state name.
    std::string getName() const override;

    // Shows the starting gameplay message.
    void onEnter(Game &game) override;

    // Accepts movement, scroll use, restart, and quit commands.
    void handleCommand(Game &game, const std::string &command) override;
};

// Concrete State: the run has ended in victory.
class WonState: public GameState {
  public:
    // Returns the victory state name.
    std::string getName() const override;

    // Shows the victory prompt.
    void onEnter(Game &game) override;

    // Accepts only restart and quit after winning.
    void handleCommand(Game &game, const std::string &command) override;
};

// Concrete State: the run has ended in defeat.
class LostState: public GameState {
  public:
    // Returns the defeat state name.
    std::string getName() const override;

    // Shows the defeat prompt.
    void onEnter(Game &game) override;

    // Accepts only restart and quit after defeat.
    void handleCommand(Game &game, const std::string &command) override;
};

#endif
