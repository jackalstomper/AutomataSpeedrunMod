#pragma once

#include <cstdint>

namespace AutomataMod {

class ChipManager
{
public:
    struct ChipSlot
    {
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

private:
    ChipSlot* const _firstChip; // The memory address that the chip table starts at
    int32_t* _money;

    static const int MAX_SLOT_COUNT; // total amount of chips that can be stored on a player profile

    // Empties the chip slot
    void reset(ChipSlot* chip);
public:
    static const uint32_t EMPTY_SLOT_ID; // chip ID indicating an empty chip slot
    static const uint32_t WAUP_CHIP_ID;
    static const uint32_t RAUP_CHIP_ID;

    ChipManager(uint64_t chipTableRamStart, int32_t* money);
    ChipSlot* getChipSlotById(uint32_t chipId);
    void addEndingEChips();
};

} // namespace AutomataMod