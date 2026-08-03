#ifndef HOSTILITYSTRATEGY_H
#define HOSTILITYSTRATEGY_H

#include <string>

#include "Position.h"

class AbstractSpectre;

// HostilityContext gives strategies access to facts that affect hostility.
// Vault Anchor strategies use this context to ask about guarded items.
class HostilityContext {
  public:
    virtual ~HostilityContext() = default;

    // Returns the Arcanist's current position.
    virtual Position getArcanistPosition() const = 0;

    // Returns whether the Arcanist is adjacent to the item guarded by this anchor.
    virtual bool isArcanistAdjacentToGuardedItem(const Position &anchorPosition) const;
};

// ==================== DESIGN PATTERN: Strategy ====================
// HostilityStrategy decides whether a spectre should attack instead of move.
// This prevents Spectre/Game from hard-coding every enemy-specific hostility rule.
class HostilityStrategy {
  public:
    virtual ~HostilityStrategy() = default;

    // Returns the strategy name for tests/report discussion.
    virtual std::string getName() const = 0;

    // Returns true when this spectre is currently hostile.
    virtual bool isHostile(const AbstractSpectre &spectre, const HostilityContext &context) const = 0;
};

// Concrete Strategy: hostile whenever the Arcanist is adjacent.
class AlwaysHostile: public HostilityStrategy {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Returns true when the Arcanist is within one-block radius.
    bool isHostile(const AbstractSpectre &spectre, const HostilityContext &context) const override;
};

// Concrete Strategy: never hostile yet.
class NeutralHostility: public HostilityStrategy {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Always returns false.
    bool isHostile(const AbstractSpectre &spectre, const HostilityContext &context) const override;
};

// Concrete Strategy: hostile only when the Arcanist is adjacent to the guarded item.
class GuardedItemHostility: public HostilityStrategy {
  public:
    // Returns the strategy name.
    std::string getName() const override;

    // Returns true when this anchor's guarded item is threatened.
    bool isHostile(const AbstractSpectre &spectre, const HostilityContext &context) const override;
};

#endif
