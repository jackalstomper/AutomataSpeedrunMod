module;

#include <cstdint>
#include <vector>

export module InventoryManager;

import PointerIterator;

namespace AutomataMod {

// Each slot is a 4 byte ID, followed by 4 bytes of ???, followed by a 4 byte count of that item
export struct InventoryItem {
    uint32_t itemId;
    uint32_t unknown;
    uint32_t quantity;

    void reset() {
        itemId = ~0u;
        unknown = ~0u;
        quantity = 0;
    }
};

static_assert(std::is_trivially_copyable<InventoryItem>::value, "InventoryItem must be POD");
static_assert(sizeof(InventoryItem) == 12, "InventoryItem isn't 12 bytes! This breaks pointer logic with game memory reading!");

export class InventoryManager {
public:
    static const int MAX_SLOT_COUNT = 255;
    static const uint32_t SEVERED_CABLE_ID = 550;
    static const uint32_t DENTED_PLATE_ID = 610;
    static const uint32_t EMPTY_SLOT_ID = 0xFFFFFFFF;

    static const uint32_t FISH_AROWANA_ID = 8001;
    static const uint32_t FISH_MACKEREL_ID = 8016;
    static const uint32_t FISH_BROKEN_FIREARM_ID = 8041;

    using Iterator = PointerIterator<InventoryItem>;

private:
    InventoryItem* const _firstSlot; // The memory address that the item table starts at
    static const int MAX_SLOT_COUNT; // the total number of inventory slots the game supports

public:
    InventoryManager(uint64_t itemTableRamStart) : _firstSlot(reinterpret_cast<InventoryItem*>(itemTableRamStart)) {}

    // Returns the slot for the given index, or nullptr if not found
    Iterator getItemSlotById(uint32_t itemId) {
        for (auto i = begin(); i != end(); ++i) {
            if (i->itemId == itemId)
                return i;
        }

        return end();
    }

    // Returns all items that match the given item ID range (inclusive)
    std::vector<Iterator> getAllItemsByRange(uint32_t itemIdStart, uint32_t itemIdEnd) {
        std::vector<Iterator> items;
        for (auto i = begin(); i != end(); ++i) {
            if (i->itemId >= itemIdStart && i->itemId <= itemIdEnd)
                items.push_back(i);
        }

        return items;
    }

    void addItem(const InventoryItem& slot) {
        Iterator emptySlot = getItemSlotById(EMPTY_SLOT_ID);
        if (emptySlot != end()) {
            emptySlot->itemId = slot.itemId;
            emptySlot->unknown = slot.unknown;
            emptySlot->quantity = slot.quantity;
        }
    }

    void removeItem(Iterator slot) {
        slot->itemId = EMPTY_SLOT_ID;
        slot->quantity = 0;
        slot->unknown = ~0u;
    }

    Iterator begin() {
        return Iterator(_firstSlot);
    }

    Iterator end() {
        return Iterator(_firstSlot + MAX_SLOT_COUNT);
    }
};

} // namespace AutomataMod