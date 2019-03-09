#pragma once
#include <cstdint>

namespace AutomataMod {

class InventoryManager
{
    uint32_t* _itemTableRamStart; // The memory address that the item table starts at
    static const int MAX_SLOT_COUNT; // the total number of inventory slots the game supports
    static const int SLOT_SIZE; // Size of each inventory slot in DWORD's

public:
    static const uint32_t SEVERED_CABLE_ID;
    static const uint32_t DENTED_PLATE_ID;
    static const uint32_t EMPTY_SLOT_ID;

    // Each slot is a 4 byte ID, followed by 4 bytes of ???, followed by a 4 byte count of that item
    struct ItemSlot
    {
        uint32_t* itemId;
        uint32_t* unknown;
        uint32_t* quantity;
    };

    InventoryManager(uint32_t* itemTableRamStart);

    // Gets item slot for given slot index
    ItemSlot getItemSlot(int slotNumber);

    // Returns the slot index for an item that has the given ID, or -1 if not found
    int getItemSlotIndexById(uint32_t itemId);

    void setVc3Inventory();
};

} // namespace AutomataMod