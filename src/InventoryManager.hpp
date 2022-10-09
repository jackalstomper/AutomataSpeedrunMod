#pragma once

#include "infra/PointerIterator.hpp"
#include "infra/defs.hpp"
#include <cstdint>
#include <vector>

namespace AutomataMod {

namespace Inventory {

extern const u32 SEVERED_CABLE_ID;
extern const u32 DENTED_PLATE_ID;
extern const u32 EMPTY_SLOT_ID;

extern const u32 FISH_AROWANA_ID;
extern const u32 FISH_MACKEREL_ID;
extern const u32 FISH_BROKEN_FIREARM_ID;
extern const u32 MAX_SLOT_COUNT; // the total number of inventory slots the game supports

struct Item {
	u32 itemId;
	u32 unknown;
	u32 quantity;

	void reset();
};

class Manager {
	Item *_firstSlot; // The memory address that the item table starts at

public:
	using Iterator = PointerIterator<Item>;
	Manager();
	Manager(Item *itemTableStart);

	// Returns the slot for the given index, or nullptr if not found
	Iterator getItemSlotById(u32 itemId);

	// Returns all items that match the given item ID range (inclusive)
	std::vector<Iterator> getAllItemsByRange(u32 itemIdStart, u32 itemIdEnd);

	void addItem(const Item &slot);

	void removeItem(Iterator slot);

	Iterator begin();

	Iterator end();
};

} // namespace Inventory

} // namespace AutomataMod
