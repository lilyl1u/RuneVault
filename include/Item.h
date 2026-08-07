#ifndef ITEM_H
#define ITEM_H

#include <string>

#include "Position.h"

//These enums are just used to describe the subtypes inside each class 
// Enums are reasonable because not every scroll needs its own class
enum class ItemCategory {
    Scroll,
    RuneFragment,
    MajorItem
};

enum class ScrollType {
    AttuneFocus,
    SurgePower,
    FortifyWard,
    DrainFocus,
    SapPower,
    ErodeWard
};

enum class FragmentType {
    CommonShard,
    ResonantShard,
    LichCache,
    ShardCoffer
};

enum class MajorItemType {
    CipherGem,
    AegisCloak
};

// Base class for all items that can appear on a level.
// VaultLevel owns Items through unique_ptr<Item>, so deleting a level cleans up its items.
class Item {
    Position position;

  protected:
    explicit Item(const Position &position);

  public:
    virtual ~Item() = default;

    // Returns the item's board position.
    Position getPosition() const;

    // Moves the item to an already-validated board position.
    void setPosition(const Position &newPosition);

    // Returns the broad kind of item for interaction logic.
    virtual ItemCategory getCategory() const = 0;

    // Returns the human-readable item name used in messages.
    virtual std::string getName() const = 0;

    // Returns the symbol drawn on the board.
    virtual char getSymbol() const = 0;

    // Returns the scroll subtype; base version throws for non-scrolls.
    virtual ScrollType getScrollType() const;

    // Returns the fragment subtype; base version throws for non-fragments.
    virtual FragmentType getFragmentType() const;

    // Returns the fragment value; base version throws for non-fragments.
    virtual int getFragmentValue() const;

    // Returns the major item subtype; base version throws for non-major-items.
    virtual MajorItemType getMajorItemType() const;
};

class Scroll: public Item {
    ScrollType type;

  public:
    // Creates a scroll of a specific hidden type.
    Scroll(ScrollType type, const Position &position);

    // Identifies this object as a scroll.
    ItemCategory getCategory() const override;

    // Returns the actual scroll name after it is known/used.
    std::string getName() const override;

    // Scrolls display as '?'.
    char getSymbol() const override;

    // Returns the concrete scroll effect type.
    ScrollType getScrollType() const override;
};

class RuneFragment: public Item {
    FragmentType type;
    int value;

  public:
    // Creates a rune fragment with its type and score value.
    RuneFragment(FragmentType type, int value, const Position &position);

    // Identifies this object as a rune fragment.
    ItemCategory getCategory() const override;

    // Returns the fragment type name.
    std::string getName() const override;

    // Rune fragments display as '*'.
    char getSymbol() const override;

    // Returns the concrete fragment type.
    FragmentType getFragmentType() const override;

    // Returns the score value awarded on pickup.
    int getFragmentValue() const override;
};

class MajorItem: public Item {
    MajorItemType type;

  public:
    // Creates a major item, such as the Cipher Gem or Aegis Cloak.
    MajorItem(MajorItemType type, const Position &position);

    // Identifies this object as a major item.
    ItemCategory getCategory() const override;

    // Returns the major item name.
    std::string getName() const override;

    // Major items display as their own symbols, like C or G.
    char getSymbol() const override;

    // Returns the concrete major item type.
    MajorItemType getMajorItemType() const override;
};

#endif
