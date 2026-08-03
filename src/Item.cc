#include "Item.h"

#include <stdexcept>

// Stores the shared board position for every item subtype.
Item::Item(const Position &position): position{position} {}

// Returns the item's current board position.
Position Item::getPosition() const {
    return position;
}

// Updates the item's position after caller-side validation.
void Item::setPosition(const Position &newPosition) {
    position = newPosition;
}

// Base behaviour protects against asking a non-scroll for scroll data.
ScrollType Item::getScrollType() const {
    throw std::logic_error{"Item is not a scroll"};
}

// Base behaviour protects against asking a non-fragment for fragment data.
FragmentType Item::getFragmentType() const {
    throw std::logic_error{"Item is not a rune fragment"};
}

// Base behaviour protects against asking a non-fragment for score value.
int Item::getFragmentValue() const {
    throw std::logic_error{"Item is not a rune fragment"};
}

// Base behaviour protects against asking a non-major-item for major item data.
MajorItemType Item::getMajorItemType() const {
    throw std::logic_error{"Item is not a major item"};
}

// Creates a scroll with its hidden effect type.
Scroll::Scroll(ScrollType type, const Position &position):
    Item{position},
    type{type} {}

// Identifies this item as a scroll.
ItemCategory Scroll::getCategory() const {
    return ItemCategory::Scroll;
}

// Converts the scroll type into a user-facing name.
std::string Scroll::getName() const {
    switch (type) {
        case ScrollType::AttuneFocus:
            return "Attune Focus";
        case ScrollType::SurgePower:
            return "Surge Power";
        case ScrollType::FortifyWard:
            return "Fortify Ward";
        case ScrollType::DrainFocus:
            return "Drain Focus";
        case ScrollType::SapPower:
            return "Sap Power";
        case ScrollType::ErodeWard:
            return "Erode Ward";
    }

    return "Unknown Scroll";
}

// Returns the shared display symbol for all scrolls.
char Scroll::getSymbol() const {
    return '?';
}

// Returns the concrete scroll effect type.
ScrollType Scroll::getScrollType() const {
    return type;
}

// Creates a rune fragment with its scoring value.
RuneFragment::RuneFragment(FragmentType type, int value, const Position &position):
    Item{position},
    type{type},
    value{value} {}

// Identifies this item as a rune fragment.
ItemCategory RuneFragment::getCategory() const {
    return ItemCategory::RuneFragment;
}

// Converts the fragment type into a user-facing name.
std::string RuneFragment::getName() const {
    switch (type) {
        case FragmentType::CommonShard:
            return "Common Shard";
        case FragmentType::ResonantShard:
            return "Resonant Shard";
        case FragmentType::LichCache:
            return "Lich Cache";
        case FragmentType::ShardCoffer:
            return "Shard Coffer";
    }

    return "Unknown Rune Fragment";
}

// Returns the shared display symbol for all rune fragments.
char RuneFragment::getSymbol() const {
    return '*';
}

// Returns the concrete rune fragment type.
FragmentType RuneFragment::getFragmentType() const {
    return type;
}

// Returns how much score this fragment is worth.
int RuneFragment::getFragmentValue() const {
    return value;
}

// Creates a major item such as the Cipher Gem or Aegis Cloak.
MajorItem::MajorItem(MajorItemType type, const Position &position):
    Item{position},
    type{type} {}

// Identifies this item as a major item.
ItemCategory MajorItem::getCategory() const {
    return ItemCategory::MajorItem;
}

// Converts the major item type into a user-facing name.
std::string MajorItem::getName() const {
    switch (type) {
        case MajorItemType::CipherGem:
            return "Cipher Gem";
        case MajorItemType::AegisCloak:
            return "Aegis Cloak";
    }

    return "Unknown Major Item";
}

// Returns the unique display symbol for the major item type.
char MajorItem::getSymbol() const {
    if (type == MajorItemType::CipherGem) {
        return 'C';
    }

    return 'G';
}

// Returns the concrete major item type.
MajorItemType MajorItem::getMajorItemType() const {
    return type;
}
