#include "InventoryManager.hpp"
#include "Log.hpp"
#include <type_traits>

namespace AutomataMod {

static_assert(std::is_pod<InventoryManager::Item>::value, "ItemSlot must be POD");
static_assert(sizeof(InventoryManager::Item) == 12, "ItemSlot isn't 12 bytes! This breaks pointer logic with game memory reading!");

const int InventoryManager::MAX_SLOT_COUNT = 255;
const uint32_t InventoryManager::SEVERED_CABLE_ID = 550;
const uint32_t InventoryManager::DENTED_PLATE_ID = 610;
const uint32_t InventoryManager::EMPTY_SLOT_ID = 0xFFFFFFFF;

const uint32_t InventoryManager::FISH_AROWANA_ID = 8001;
const uint32_t InventoryManager::FISH_MACKEREL_ID = 8016;
const uint32_t InventoryManager::FISH_BROKEN_FIREARM_ID = 8041;

void InventoryManager::Item::reset()
{
    itemId = InventoryManager::EMPTY_SLOT_ID;
    unknown = ~0u;
    quantity = 0;
}

InventoryManager::InventoryManager(uint64_t itemTableRamStart) : _firstSlot(reinterpret_cast<Item*>(itemTableRamStart))
{}

InventoryManager::Iterator InventoryManager::getItemSlotById(uint32_t itemId)
{
    for (auto i = begin(); i != end(); ++i) {
        if (i->itemId == itemId)
            return i;
    }

    return end();
}

std::vector<InventoryManager::Iterator> InventoryManager::getAllItemsByRange(uint32_t itemIdStart, uint32_t itemIdEnd)
{
    std::vector<Iterator> items;
    for (auto i = begin(); i != end(); ++i) {
        if (i->itemId >= itemIdStart && i->itemId <= itemIdEnd)
            items.push_back(i);
    }

    return items;
}

void InventoryManager::addItem(const Item& slot)
{
    Iterator emptySlot = getItemSlotById(EMPTY_SLOT_ID);
    if (emptySlot != end()) {
        emptySlot->itemId = slot.itemId;
        emptySlot->unknown = slot.unknown;
        emptySlot->quantity = slot.quantity;
    }
}

void InventoryManager::removeItem(Iterator slot)
{
    slot->itemId = EMPTY_SLOT_ID;
    slot->quantity = 0;
    slot->unknown = ~0u;
}

InventoryManager::Iterator InventoryManager::begin()
{
    return Iterator(_firstSlot);
}

InventoryManager::Iterator InventoryManager::end()
{
    return Iterator(_firstSlot + MAX_SLOT_COUNT);
}

} // namespace AutomataMod