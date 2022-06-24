module;

#include <cstdint>
#include <type_traits>
#include <algorithm>

export module ChipManager;

import PointerIterator;

namespace AutomataMod {

export struct ChipSlot {
    uint32_t unknown0; // starts at 0, changes based on chip in unknown ways...
    uint32_t id; // starts at 3001, increments for every chip + all their supported levels

    uint32_t chipIndex; // an index for each type of chip there is in the game (not including +1/2/3)
    uint32_t chipLevel; // The chip boost level (+0, +1, +2, etc)
    uint32_t slotCost; // Amount of slots the chip takes up in a set

    // position of the chip in the chip sets. Set to 0xFFFFFFFF to not assign to set
    uint32_t setAPos;
    uint32_t setBPos;
    uint32_t setCPos;

    uint32_t unknown1; // Always 0xFFFFFFFF (except OS chip, which is 0)
    uint32_t unknown2; // Always 0xFFFFFFFF (except OS chip, which is 0)
    uint32_t unknown3; // Always 0xFFFFFFFF (except OS chip, which is 0)
    uint32_t unknown4; // Always 0
};

export class ChipManager {
public:

    using Iterator = PointerIterator<ChipSlot>;

private:
    ChipSlot* const _firstChip; // The memory address that the chip table starts at
    static const int MAX_SLOT_COUNT = 300; // total amount of chips that can be stored on a player profile

public:
    static const uint32_t EMPTY_SLOT_ID = ~0; // chip ID indicating an empty chip slot
    static const uint32_t WAUP_CHIP_ID = 3007;
    static const uint32_t RAUP_CHIP_ID = 3034;
    static const uint32_t TAUNT2_CHIP_ID = 3228;

    ChipManager(uint64_t chipTableRamStart) : _firstChip(reinterpret_cast<ChipSlot*>(chipTableRamStart))
    {}

    // Returns a chip with the given chip ID present in inventory, or nullptr if none found
    Iterator getChipSlotById(uint32_t chipId) {
        return std::find_if(begin(), end(), [chipId](const ChipSlot& i) { return i.id == chipId; });
    }

    void addChip(const ChipSlot& newChip) {
        Iterator slot = getChipSlotById(EMPTY_SLOT_ID);
        if (slot != end())
            *slot = newChip;
    }

    Iterator begin() {
        return Iterator(_firstChip);
    }

    Iterator end() {
        return Iterator(_firstChip + MAX_SLOT_COUNT);
    }
};

static_assert(std::is_trivially_copyable<ChipSlot>::value, "ChipSlot must be POD");
static_assert(sizeof(ChipSlot) == 48, "ChipSlot isn't 48 bytes in size! This breaks pointer reading logic with game memory!");

} // namespace AutomataMod