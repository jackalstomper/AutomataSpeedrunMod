#include <cstdint>
#include <chrono>
#include <thread>
#include "InventoryManager.hpp"

namespace {

// Addresses are offsets in bytes relevant to processRamStart
const uint64_t CURRENT_PHASE_ADDR = 0x1101D58;
const uint64_t PLAYER_SET_NAME_ADDR = 0x147B4BC;
const uint64_t IS_WORLD_LOADED_ADDR = 0x110ADC0;
const uint64_t ITEM_TABLE_START_ADDR = 0x197C4C4;


// Our offsets are in bytes, so we need to compute the pointer using a byte sized type before converting into the type we desire
template<typename T>
T getOffsetPointer(uint8_t* processRamStart, uint64_t address)
{
    return reinterpret_cast<T>(processRamStart + address);
}

} // namespace

namespace AutomataMod {

void checkStuff(uint8_t* processRamStart)
{
    char* currentPhase = getOffsetPointer<char*>(processRamStart, CURRENT_PHASE_ADDR);
    uint32_t* playerNameSet = getOffsetPointer<uint32_t*>(processRamStart, PLAYER_SET_NAME_ADDR);
    uint32_t* isWorldLoaded = getOffsetPointer<uint32_t*>(processRamStart, IS_WORLD_LOADED_ADDR);
    uint32_t* itemTableRamStart = getOffsetPointer<uint32_t*>(processRamStart, ITEM_TABLE_START_ADDR);
    bool inventoryModded = false;

    while (true) {
        if (!inventoryModded && *isWorldLoaded == 1 && *playerNameSet == 1 && strncmp(currentPhase, "58_AB_BossArea_Fall", 19) == 0) {
            AutomataMod::InventoryManager manager(itemTableRamStart);
            manager.setVc3Inventory();
            inventoryModded = true;
        }

        if (*isWorldLoaded == 0 && *playerNameSet == 0) {
            if (inventoryModded)
                inventoryModded = false;
        }

        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(250)); // check stuff 4 times a second
    }
}

} // namespace AutomataMod