#include <cstring>
#include "ChipManager.hpp"

namespace AutomataMod {

static_assert(sizeof(ChipManager::ChipSlot) == 48, "ChipSlot isn't 48 bytes in size! This breaks pointer reading logic with game memory!");

const int ChipManager::MAX_SLOT_COUNT = 300;
const uint32_t ChipManager::EMPTY_SLOT_ID = ~0;
const uint32_t ChipManager::WAUP_CHIP_ID = 3007;
const uint32_t ChipManager::RAUP_CHIP_ID = 3034;
const uint32_t ChipManager::TAUNT2_CHIP_ID = 3228;

ChipManager::ChipSlot& ChipManager::ChipSlot::operator=(const ChipSlot& other)
{
    unknown0 = other.unknown0;
    id = other.id;
    chipIndex = other.chipIndex;
    chipLevel = other.chipLevel;
    slotCost = other.slotCost;
    setAPos = other.setAPos;
    setBPos = other.setBPos;
    setCPos = other.setCPos;
    unknown1 = other.unknown1;
    unknown2 = other.unknown2;
    unknown3 = other.unknown3;
    unknown4 = other.unknown4;
    return *this;
}

ChipManager::Iterator ChipManager::begin()
{
    return Iterator(_firstChip);
}

ChipManager::Iterator ChipManager::end()
{
    return Iterator(_firstChip + MAX_SLOT_COUNT);
}

ChipManager::ChipManager(uint64_t chipTableRamStart) : _firstChip(reinterpret_cast<ChipSlot*>(chipTableRamStart))
{}

ChipManager::Iterator ChipManager::getChipSlotById(uint32_t chipId)
{
    return std::find_if(begin(), end(), [chipId](const ChipSlot& i) { return i.id == chipId; });
}

void ChipManager::addChip(const ChipSlot& newChip)
{
    Iterator slot = getChipSlotById(EMPTY_SLOT_ID);
    if (slot != end())
        *slot = newChip;
}

} // namespace AutomataMod