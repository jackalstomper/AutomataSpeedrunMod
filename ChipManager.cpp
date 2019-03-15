#include <cstring>
#include "ChipManager.hpp"

namespace AutomataMod {

static_assert(sizeof(ChipManager::ChipSlot) == 48, "ChipSlot isn't 48 bytes in size! This breaks pointer reading logic with game memory!");

const int ChipManager::MAX_SLOT_COUNT = 300;
const uint32_t ChipManager::EMPTY_SLOT_ID = 0xFFFFFFFF;
const uint32_t ChipManager::WAUP_CHIP_ID = 3007;
const uint32_t ChipManager::RAUP_CHIP_ID = 3034;

void ChipManager::reset(ChipSlot* slot)
{
    if (!slot)
        return;

    memset(slot, 0xFF, sizeof(ChipSlot));
    slot->unknown4 = 0;
}

ChipManager::ChipManager(uint64_t chipTableRamStart, int32_t* money) : _firstChip(reinterpret_cast<ChipSlot*>(chipTableRamStart)), _money(money)
{}

ChipManager::ChipSlot* ChipManager::getChipSlotById(uint32_t chipId)
{
    for (ChipSlot* i = _firstChip; i != _firstChip + MAX_SLOT_COUNT; ++i) {
        if (i->id == chipId)
            return i;
    }

    return nullptr;
}

void ChipManager::addEndingEChips()
{
    ChipSlot* waupSlot1 = nullptr;
    ChipSlot* waupSlot2 = nullptr;
    ChipSlot* raupSlot = nullptr;

    for (ChipSlot* i = _firstChip; i != _firstChip + MAX_SLOT_COUNT; ++i) {
        if (i->id == WAUP_CHIP_ID) {
            if (!waupSlot1)
                waupSlot1 = i;
            else if (!waupSlot2)
                waupSlot2 = i;
        } else if (i->id == RAUP_CHIP_ID && !raupSlot) {
            raupSlot = i;
        }

        if (waupSlot1 && waupSlot2 && raupSlot)
            break;
    }

    *_money -= 30000;
    if (*_money < 0)
        *_money = 0;

    if (!raupSlot && (raupSlot = getChipSlotById(EMPTY_SLOT_ID)) != nullptr) {
        reset(raupSlot);
        raupSlot->unknown0 = 33;
        raupSlot->id = RAUP_CHIP_ID;
        raupSlot->chipIndex = 4;
        raupSlot->chipLevel = 6;
        raupSlot->slotCost = 28;
    }

    if (!waupSlot1 && (waupSlot1 = getChipSlotById(EMPTY_SLOT_ID)) != nullptr) {
        reset(waupSlot1);
        waupSlot1->unknown0 = 6;
        waupSlot1->id = WAUP_CHIP_ID;
        waupSlot1->chipLevel = 1;
        waupSlot1->slotCost = 28;
    }

    if (!waupSlot2 && (waupSlot2 = getChipSlotById(EMPTY_SLOT_ID)) != nullptr) {
        reset(waupSlot2);
        waupSlot2->unknown0 = 6;
        waupSlot2->id = WAUP_CHIP_ID;
        waupSlot2->chipLevel = 1;
        waupSlot2->slotCost = 28;
    }
}

} // namespace AutomataMod