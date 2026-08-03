#ifndef ITEMFACTORY_H
#define ITEMFACTORY_H

#include <memory>

#include "Item.h"

// ==================== DESIGN PATTERN: Factory Method / Simple Factory ====================
// ItemFactory keeps item construction in one place, matching the UML goal that
// Scrolls, RuneFragments, and MajorItems share a common creation interface.
class ItemFactory {
  public:
    // Creates a scroll and transfers ownership to the caller.
    std::unique_ptr<Item> createScroll(ScrollType type, const Position &position) const;

    // Creates a rune fragment with the correct score value.
    std::unique_ptr<Item> createFragment(FragmentType type, const Position &position) const;

    // Creates a major item and transfers ownership to the caller.
    std::unique_ptr<Item> createMajorItem(MajorItemType type, const Position &position) const;
};

#endif
