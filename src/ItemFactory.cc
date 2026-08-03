#include "ItemFactory.h"

// Creates a concrete Scroll while returning it through the Item interface.
std::unique_ptr<Item> ItemFactory::createScroll(
    ScrollType type,
    const Position &position) const {
    return std::make_unique<Scroll>(type, position);
}

// Creates a concrete RuneFragment with the score value required by the spec.
std::unique_ptr<Item> ItemFactory::createFragment(
    FragmentType type,
    const Position &position) const {
    int value = 1;

    if (type == FragmentType::ResonantShard) {
        value = 2;
    } else if (type == FragmentType::LichCache) {
        value = 4;
    } else if (type == FragmentType::ShardCoffer) {
        value = 6;
    }

    return std::make_unique<RuneFragment>(type, value, position);
}

// Creates a concrete MajorItem while returning it through the Item interface.
std::unique_ptr<Item> ItemFactory::createMajorItem(
    MajorItemType type,
    const Position &position) const {
    return std::make_unique<MajorItem>(type, position);
}
