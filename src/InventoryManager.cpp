#include "InventoryManager.hpp"

#include <type_traits>

namespace AutomataMod {

namespace Inventory {

const u32 MAX_SLOT_COUNT = 255; // the total number of inventory slots the game supports

const u32 SEVERED_CABLE_ID = 550;
const u32 DENTED_PLATE_ID = 610;
const u32 EMPTY_SLOT_ID = 0xFFFFFFFF;

const u32 FISH_AROWANA_ID = 8001;
const u32 FISH_MACKEREL_ID = 8016;
const u32 FISH_BROKEN_FIREARM_ID = 8041;

void Item::reset() {
	itemId = ~0u;
	unknown = ~0u;
	quantity = 0;
}

Manager::Manager() : _firstSlot(nullptr) {}
Manager::Manager(Item *itemTableStart) : _firstSlot(itemTableStart) {}

// Returns the slot for the given index, or nullptr if not found
Manager::Iterator Manager::getItemSlotById(u32 itemId) {
	for (auto i = begin(); i != end(); ++i) {
		if (i->itemId == itemId)
			return i;
	}

	return end();
}

// Returns all items that match the given item ID range (inclusive)
std::vector<Manager::Iterator> Manager::getAllItemsByRange(u32 itemIdStart, u32 itemIdEnd) {
	std::vector<Iterator> items;
	for (auto i = begin(); i != end(); ++i) {
		if (i->itemId >= itemIdStart && i->itemId <= itemIdEnd)
			items.push_back(i);
	}

	return items;
}

void Manager::addItem(const Item &slot) {
	Iterator emptySlot = getItemSlotById(EMPTY_SLOT_ID);
	if (emptySlot != end()) {
		emptySlot->itemId = slot.itemId;
		emptySlot->unknown = slot.unknown;
		emptySlot->quantity = slot.quantity;
	}
}

void Manager::removeItem(Iterator slot) {
	slot->itemId = EMPTY_SLOT_ID;
	slot->quantity = 0;
	slot->unknown = ~0u;
}

Manager::Iterator Manager::begin() { return Iterator(_firstSlot); }

Manager::Iterator Manager::end() { return Iterator(_firstSlot + MAX_SLOT_COUNT); }

static_assert(std::is_trivially_copyable<Item>::value, "InventoryItem must be POD");
static_assert(sizeof(Item) == 12, "InventoryItem isn't 12 bytes! This breaks pointer logic with game memory reading!");

} // namespace Inventory

} // namespace AutomataMod
