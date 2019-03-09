#include <cstring>
#include "ChipManager.hpp"

namespace AutomataMod {

const int ChipManager::MAX_SLOT_COUNT = 300;
const int ChipManager::SLOT_SIZE = 12;
const uint32_t ChipManager::EMPTY_SLOT_ID = 0xFFFFFFFF;
const uint32_t ChipManager::WAUP_CHIP_ID = 3007;
const uint32_t ChipManager::RAUP_CHIP_ID = 3034;

ChipManager::ChipSlot::ChipSlot(uint32_t* startPos) : 
    unknown0(startPos), 
    id(startPos + 1),
    chipIndex(startPos + 2),
    chipLevel(startPos + 3),
    slotCost(startPos + 4),
    setAPos(startPos + 5),
    setBPos(startPos + 6),
    setCPos(startPos + 7),
    unknown1(startPos + 8),
    unknown2(startPos + 9),
    unknown3(startPos + 10),
    unknown4(startPos + 11)
{}

void ChipManager::ChipSlot::reset()
{
    memset(unknown0, 0xFF, SLOT_SIZE * 4);
    *unknown4 = 0;
}

ChipManager::ChipManager(uint32_t* chipTableRamStart, int32_t* money) : _chipTableRamStart(chipTableRamStart), _money(money)
{}

ChipManager::ChipSlot ChipManager::getChipSlotBySlotIndex(int slotNumber)
{
    uint32_t* slotAddress = _chipTableRamStart + (slotNumber * SLOT_SIZE);
    return ChipSlot(slotAddress);
}

int ChipManager::getChipSlotIndexById(uint32_t chipId)
{
    uint32_t iterId;
    for (int i = 0; i < MAX_SLOT_COUNT; ++i) {
        iterId = *(_chipTableRamStart + (i * SLOT_SIZE) + 1); // + 1 to skip the unknown field at beginning of each slot
        if (iterId == chipId)
            return i;
    }

    return -1;
}

void ChipManager::addEndingEChips()
{
    int waupSlotIndex1 = -1;
    int waupSlotIndex2 = -1;
    int raupSlotIndex = -1;

    uint32_t chipId;
    for (int i = 0; i < MAX_SLOT_COUNT; ++i) {
        chipId = *(_chipTableRamStart + (i * SLOT_SIZE) + 1); // + 1 to skip the unknown field at beginning of each slot
        if (chipId == WAUP_CHIP_ID) {
            if (waupSlotIndex1 == -1)
                waupSlotIndex1 = i;
            else if (waupSlotIndex2 == -1)
                waupSlotIndex2 = i;
        } else if (chipId == RAUP_CHIP_ID && raupSlotIndex == -1) {
            raupSlotIndex = i;
        }

        if (raupSlotIndex > -1 && waupSlotIndex1 > -1 && waupSlotIndex2 > -1)
            break;
    }

    *_money -= 30000;
    if (*_money < 0)
        *_money = 0;

    if (raupSlotIndex == -1 && (raupSlotIndex = getChipSlotIndexById(EMPTY_SLOT_ID)) != -1) {
        ChipSlot slot = getChipSlotBySlotIndex(raupSlotIndex);
        slot.reset();
        *slot.unknown0 = 33;
        *slot.id = RAUP_CHIP_ID;
        *slot.chipIndex = 4;
        *slot.chipLevel = 6;
        *slot.slotCost = 28;
    }

    if (waupSlotIndex1 == -1 && (waupSlotIndex1 = getChipSlotIndexById(EMPTY_SLOT_ID)) != -1) {
        ChipSlot slot = getChipSlotBySlotIndex(waupSlotIndex1);
        slot.reset();
        *slot.unknown0 = 6;
        *slot.id = WAUP_CHIP_ID;
        *slot.chipIndex = 1;
        *slot.chipLevel = 6;
        *slot.slotCost = 28;
    }

    if (waupSlotIndex2 == -1 && (waupSlotIndex2 = getChipSlotIndexById(EMPTY_SLOT_ID)) != -1) {
        ChipSlot slot = getChipSlotBySlotIndex(waupSlotIndex2);
        slot.reset();
        *slot.unknown0 = 6;
        *slot.id = WAUP_CHIP_ID;
        *slot.chipIndex = 1;
        *slot.chipLevel = 6;
        *slot.slotCost = 28;
    }
}

} // namespace AutomataMod