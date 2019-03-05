#include "InventoryManager.hpp"

namespace AutomataMod {

const int InventoryManager::MAX_SLOT_COUNT = 512;
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

int InventoryManager::getEmptyItemSlot()
{
    uint32_t itemId;
    for (int i = 0; i < MAX_SLOT_COUNT; ++i) {
        itemId = *(_itemTableRamStart + (i * SLOT_SIZE));
        if (itemId == EMPTY_SLOT_ID)
            return i;
    }

    return -1;
}

void InventoryManager::setVc3Inventory()
{
    ItemSlot dentedPlateSlot;
    ItemSlot severedCableSlot;
    uint32_t itemId;
    bool foundSeveredCable = false;
    bool foundDentedPlate = false;

    for (int i = 0; i < MAX_SLOT_COUNT; ++i) {
        itemId = *(_itemTableRamStart + (i * SLOT_SIZE));

        if (itemId == DENTED_PLATE_ID) {
            dentedPlateSlot = getItemSlot(i);
            foundDentedPlate = true;
        } else if (itemId == SEVERED_CABLE_ID) {
            severedCableSlot = getItemSlot(i);
            foundSeveredCable = true;
        }

        if (foundSeveredCable && foundDentedPlate)
            break;
    }

    // In order to get VC3 after adam pit we need:
    // 4 dented plates
    // 3 severed cables

    if (foundDentedPlate) {
        // already have dented plate, only add nessesary amount to continue run
        if (*dentedPlateSlot.quantity < 4)
            *dentedPlateSlot.quantity = 4;
    } else {
        // need to add invetnory for dented plate
        int emptySlot = getEmptyItemSlot();
        if (emptySlot == -1)
            return; // We dont have any inventory room left and we didn't find any dented plates. Something's gone wrong.

        dentedPlateSlot = getItemSlot(emptySlot);
        *dentedPlateSlot.itemId = DENTED_PLATE_ID;
        *dentedPlateSlot.unknown = 0xFFFFFFFF;
        *dentedPlateSlot.quantity = 4;
    }

    if (foundSeveredCable) {
        // already have severed cable, only add nessesary amount to continue run
        if (*severedCableSlot.quantity < 3)
            *severedCableSlot.quantity = 3;
    } else {
        // need to add inventory for severed cable
        int emptySlot = getEmptyItemSlot();
        if (emptySlot == -1)
            return; // We dont have any inventory room left and we didn't find any severed cable. Something's gone wrong.

        severedCableSlot = getItemSlot(emptySlot);
        *severedCableSlot.itemId = SEVERED_CABLE_ID;
        *severedCableSlot.unknown = 0xFFFFFFFF;
        *severedCableSlot.quantity = 3;
    }
}

} // namespace AutomataMod