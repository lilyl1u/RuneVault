#ifndef SPECTRE_H
#define SPECTRE_H

#include <string>
#include <memory>
#include <optional>

#include "HostilityStrategy.h"
#include "MovementStrategy.h"
#include "Position.h"

enum class SpectreType {
    Wraith,
    Banshee,
    Revenant,
    Shade,
    Lich,
    VaultAnchor,
    SpecterLord
};

// AbstractSpectre stores the state shared by all enemy types.
// Concrete subclasses supply only the stats, symbol, and type.
class AbstractSpectre {
    SpectreType type;
    std::string name;
    char symbol;
    int focusPoints;
    int power;
    int ward;
    Position position;
    int spawnChamberId;
    std::unique_ptr<MovementStrategy> movementStrategy;
    std::unique_ptr<HostilityStrategy> hostilityStrategy;
    bool cipherGemCarrier;

  protected:
    // Initializes shared spectre state from the project specification.
    AbstractSpectre(
        SpectreType type,
        const std::string &name,
        char symbol,
        int focusPoints,
        int power,
        int ward,
        const Position &position,
        int spawnChamberId,
        std::unique_ptr<MovementStrategy> movementStrategy,
        std::unique_ptr<HostilityStrategy> hostilityStrategy);

  public:
    virtual ~AbstractSpectre() = default;

    // Returns the concrete spectre type.
    SpectreType getType() const;

    // Returns the user-facing spectre name.
    std::string getName() const;

    // Returns the board symbol.
    char getSymbol() const;

    // Returns current Focus Points.
    int getFocusPoints() const;

    // Returns true when Focus Points are 0.
    bool isDefeated() const;

    // Returns attack Power.
    int getPower() const;

    // Returns Ward Rating.
    int getWard() const;

    // Returns current board position.
    Position getPosition() const;

    // Returns the chamber this spectre spawned in.
    int getSpawnChamberId() const;

    // Returns the movement strategy name.
    std::string getMovementStrategyName() const;

    // Returns the hostility strategy name.
    std::string getHostilityStrategyName() const;

    // Returns whether this spectre carries the Cipher Gem.
    bool hasCipherGem() const;

    // Asks the movement strategy to choose a move destination.
    std::optional<Position> chooseMove(const MovementContext &context, Random &rng) const;

    // Asks the hostility strategy whether this spectre should attack.
    bool isHostile(const HostilityContext &context) const;

    // Moves the spectre to an already-validated board position.
    void setPosition(const Position &newPosition);

    // Reduces Focus Points while preventing negative values.
    void takeDamage(int damage);

    // Marks whether this spectre carries the Cipher Gem.
    void setCipherGemCarrier(bool carriesCipherGem);
};

class Wraith: public AbstractSpectre {
  public:
    // Creates a Wraith with the spec's starting stats.
    Wraith(const Position &position, int spawnChamberId);
};

class Banshee: public AbstractSpectre {
  public:
    // Creates a Banshee with the spec's starting stats.
    Banshee(const Position &position, int spawnChamberId);
};

class Revenant: public AbstractSpectre {
  public:
    // Creates a Revenant with the spec's starting stats.
    Revenant(const Position &position, int spawnChamberId);
};

class Shade: public AbstractSpectre {
  public:
    // Creates a Shade with the spec's starting stats.
    Shade(const Position &position, int spawnChamberId);
};

class Lich: public AbstractSpectre {
  public:
    // Creates a Lich with the spec's starting stats.
    Lich(const Position &position, int spawnChamberId, bool hostile = false);
};

class VaultAnchor: public AbstractSpectre {
  public:
    // Creates a stationary Vault Anchor with the spec's starting stats.
    VaultAnchor(const Position &position, int spawnChamberId);
};

class SpecterLord: public AbstractSpectre {
  public:
    // Creates a Specter Lord with the spec's starting stats.
    SpecterLord(const Position &position, int spawnChamberId, bool enableChase = true);
};

#endif
