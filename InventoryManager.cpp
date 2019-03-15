#include "InventoryManager.hpp"

namespace AutomataMod {

static_assert(sizeof(InventoryManager::ItemSlot) == 12, "ItemSlot isn't 12 bytes! This breaks pointer logic with game memory reading!");

const int InventoryManager::MAX_SLOT_COUNT = 255;
const uint32_t InventoryManager::SEVERED_CABLE_ID = 550;
const uint32_t InventoryManager::DENTED_PLATE_ID = 610;
const uint32_t InventoryManager::EMPTY_SLOT_ID = 0xFFFFFFFF;

InventoryManager::InventoryManager(uint64_t itemTableRamStart) : _firstSlot(reinterpret_cast<ItemSlot*>(itemTableRamStart))
{}

InventoryManager::ItemSlot* InventoryManager::getItemSlotById(uint32_t itemId)
{
    for (ItemSlot* i = _firstSlot; i != _firstSlot + MAX_SLOT_COUNT; ++i) {
        if (i->itemId == itemId)
            return i;
    }

    return nullptr;
}

void InventoryManager::setVc3Inventory()
{
    ItemSlot* dentedPlateSlot = getItemSlotById(DENTED_PLATE_ID);
    ItemSlot* severedCableSlot = getItemSlotById(SEVERED_CABLE_ID);

    // In order to get VC3 after adam pit we need:
    // 4 dented plates
    // 3 severed cables
    if (!dentedPlateSlot) {
        dentedPlateSlot = getItemSlotById(EMPTY_SLOT_ID);
        if (!dentedPlateSlot)
            return; // We dont have any inventory room left and we didn't find any dented plates. Something's gone wrong.

        dentedPlateSlot->itemId = DENTED_PLATE_ID;
        dentedPlateSlot->unknown = 0xFFFFFFFF;
    }

    if (!severedCableSlot) {
        severedCableSlot = getItemSlotById(EMPTY_SLOT_ID);
        if (!severedCableSlot)
            return; // We dont have any inventory room left and we didn't find any severed cables. Something's gone wrong.

        severedCableSlot->itemId = SEVERED_CABLE_ID;
        severedCableSlot->unknown = 0xFFFFFFFF;
    }

    if (dentedPlateSlot->quantity < 4)
        dentedPlateSlot->quantity = 4;

    if (severedCableSlot->quantity < 3)
        severedCableSlot->quantity = 3;
}

} // namespace AutomataMod