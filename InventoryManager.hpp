#pragma once
#include <cstdint>
#include <vector>

namespace AutomataMod {

class InventoryManager
{
public:
    // Each slot is a 4 byte ID, followed by 4 bytes of ???, followed by a 4 byte count of that item
    struct ItemSlot
    {
        uint32_t itemId;
        uint32_t unknown;
        uint32_t quantity;
    };

    static const uint32_t SEVERED_CABLE_ID;
    static const uint32_t DENTED_PLATE_ID;
    static const uint32_t EMPTY_SLOT_ID;

    static const uint32_t FISH_AROWANA_ID;
    static const uint32_t FISH_BROKEN_FIREARM_ID;
    static const uint32_t FISH_MACKEREL_ID;

private:
    ItemSlot* const _firstSlot; // The memory address that the item table starts at
    static const int MAX_SLOT_COUNT; // the total number of inventory slots the game supports

public:
    InventoryManager(uint64_t itemTableRamStart);

    // Returns the slot for the given index, or nullptr if not found
    ItemSlot* getItemSlotById(uint32_t itemId);

    // Returns all items that match the given item ID range (inclusive)
    std::vector<ItemSlot*> getAllItemsByRange(uint32_t itemIdStart, uint32_t itemIdEnd);

    void setVc3Inventory();

    // shouldDeleteFish - true if any caught fish should be deleted from the inventory immediately
    bool adjustFishInventory(bool shouldDeleteFish);
};

} // namespace AutomataMod