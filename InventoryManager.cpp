#include "InventoryManager.hpp"

namespace AutomataMod {

const int InventoryManager::MAX_SLOT_COUNT = 512;
const int InventoryManager::MAX_SLOT_COUNT = 255;
const int InventoryManager::SLOT_SIZE = 3;
const uint32_t InventoryManager::SEVERED_CABLE_ID = 550;
const uint32_t InventoryManager::DENTED_PLATE_ID = 610;
const uint32_t InventoryManager::EMPTY_SLOT_ID = 0xFFFFFFFF;

InventoryManager::InventoryManager(uint32_t* itemTableRamStart) : _itemTableRamStart(itemTableRamStart)
{}

InventoryManager::ItemSlot InventoryManager::getItemSlot(int slotNumber)
{
    ItemSlot slot;
    uint32_t* slotStart = _itemTableRamStart + (slotNumber * SLOT_SIZE);
    slot.itemId = slotStart;
    slot.unknown = slotStart + 1;
    slot.quantity = slotStart + 2;
    return slot;
}

int InventoryManager::getItemSlotIndexById(uint32_t itemId)
{
    uint32_t iterId;
    for (int i = 0; i < MAX_SLOT_COUNT; ++i) {
        iterId = *(_itemTableRamStart + (i * SLOT_SIZE));
        if (iterId == itemId)
            return i;
    }

    return -1;
}

void InventoryManager::setVc3Inventory()
{
    ItemSlot dentedPlateSlot;
    ItemSlot severedCableSlot;
    int dentedPlateSlotIndex = getItemSlotIndexById(DENTED_PLATE_ID);
    int severedCableSlotIndex = getItemSlotIndexById(SEVERED_CABLE_ID);

    // In order to get VC3 after adam pit we need:
    // 4 dented plates
    // 3 severed cables
    if (dentedPlateSlotIndex == -1) {
        dentedPlateSlotIndex = getItemSlotIndexById(EMPTY_SLOT_ID);
        if (dentedPlateSlotIndex == -1)
            return; // We dont have any inventory room left and we didn't find any dented plates. Something's gone wrong.

        dentedPlateSlot = getItemSlot(dentedPlateSlotIndex);
        *dentedPlateSlot.itemId = DENTED_PLATE_ID;
        *dentedPlateSlot.unknown = 0xFFFFFFFF;
    } else {
        dentedPlateSlot = getItemSlot(dentedPlateSlotIndex);
    }

    if (severedCableSlotIndex == -1) {
        severedCableSlotIndex = getItemSlotIndexById(EMPTY_SLOT_ID);
        if (severedCableSlotIndex == -1)
            return; // We dont have any inventory room left and we didn't find any severed cables. Something's gone wrong.

        severedCableSlot = getItemSlot(severedCableSlotIndex);
        *severedCableSlot.itemId = SEVERED_CABLE_ID;
        *severedCableSlot.unknown = 0xFFFFFFFF;
    } else {
        severedCableSlot = getItemSlot(severedCableSlotIndex);
    }

    if (*dentedPlateSlot.quantity < 4)
        *dentedPlateSlot.quantity = 4;

    if (*severedCableSlot.quantity < 3)
        *severedCableSlot.quantity = 3;
}

} // namespace AutomataMod