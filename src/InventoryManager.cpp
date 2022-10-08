#include "InventoryManager.hpp"

#include <type_traits>

namespace AutomataMod {

using Iterator = InventoryManager::Iterator;

const int InventoryManager::MAX_SLOT_COUNT = 255; // the total number of inventory slots the game supports

const uint32_t InventoryManager::SEVERED_CABLE_ID = 550;
const uint32_t InventoryManager::DENTED_PLATE_ID = 610;
const uint32_t InventoryManager::EMPTY_SLOT_ID = 0xFFFFFFFF;

const uint32_t InventoryManager::FISH_AROWANA_ID = 8001;
const uint32_t InventoryManager::FISH_MACKEREL_ID = 8016;
const uint32_t InventoryManager::FISH_BROKEN_FIREARM_ID = 8041;

void InventoryItem::reset() {
	itemId = ~0u;
	unknown = ~0u;
	quantity = 0;
}

InventoryManager::InventoryManager(uint64_t itemTableRamStart)
		: _firstSlot(reinterpret_cast<InventoryItem *>(itemTableRamStart)) {}

// Returns the slot for the given index, or nullptr if not found
Iterator InventoryManager::getItemSlotById(uint32_t itemId) {
	for (auto i = begin(); i != end(); ++i) {
		if (i->itemId == itemId)
			return i;
	}

	return end();
}

// Returns all items that match the given item ID range (inclusive)
std::vector<Iterator> InventoryManager::getAllItemsByRange(uint32_t itemIdStart, uint32_t itemIdEnd) {
	std::vector<Iterator> items;
	for (auto i = begin(); i != end(); ++i) {
		if (i->itemId >= itemIdStart && i->itemId <= itemIdEnd)
			items.push_back(i);
	}

	return items;
}

void InventoryManager::addItem(const InventoryItem &slot) {
	Iterator emptySlot = getItemSlotById(EMPTY_SLOT_ID);
	if (emptySlot != end()) {
		emptySlot->itemId = slot.itemId;
		emptySlot->unknown = slot.unknown;
		emptySlot->quantity = slot.quantity;
	}
}

void InventoryManager::removeItem(Iterator slot) {
	slot->itemId = EMPTY_SLOT_ID;
	slot->quantity = 0;
	slot->unknown = ~0u;
}

Iterator InventoryManager::begin() { return Iterator(_firstSlot); }

Iterator InventoryManager::end() { return Iterator(_firstSlot + MAX_SLOT_COUNT); }

static_assert(std::is_trivially_copyable<InventoryItem>::value, "InventoryItem must be POD");
static_assert(sizeof(InventoryItem) == 12,
							"InventoryItem isn't 12 bytes! This breaks pointer logic with game memory reading!");

} // namespace AutomataMod
