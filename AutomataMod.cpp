#include <cstdint>
#include <chrono>
#include <thread>
#include <cstring>
#include "InventoryManager.hpp"

namespace {

// The text we will append to player names when starting a new game
const char16_t* vc3Name = u"vc3v1.2";
const size_t vc3NameLength = std::char_traits<char16_t>::length(vc3Name);

// Addresses are offsets in bytes relevant to processRamStart
const uint64_t CURRENT_PHASE_ADDR = 0x1101D58;
const uint64_t PLAYER_SET_NAME_ADDR = 0x147B4BC;
const uint64_t IS_WORLD_LOADED_ADDR = 0x110ADC0;
const uint64_t ITEM_TABLE_START_ADDR = 0x197C4C4;
const uint64_t PLAYER_NAME_ADDR = 0x194BF88;

} // namespace

namespace AutomataMod {

void checkStuff(uint64_t processRamStart)
{
    char* currentPhase = reinterpret_cast<char*>(processRamStart + CURRENT_PHASE_ADDR);
    uint32_t* playerNameSet = reinterpret_cast<uint32_t*>(processRamStart + PLAYER_SET_NAME_ADDR);
    uint32_t* isWorldLoaded = reinterpret_cast<uint32_t*>(processRamStart + IS_WORLD_LOADED_ADDR);

    bool inventoryModded = false;
    bool nameModded = false;
    AutomataMod::InventoryManager inventoryManager(processRamStart + ITEM_TABLE_START_ADDR);

    while (true) {
        if (!inventoryModded && *isWorldLoaded == 1 && *playerNameSet == 1 && strncmp(currentPhase, "58_AB_BossArea_Fall", 19) == 0) {
            inventoryManager.setVc3Inventory();
            inventoryModded = true;
        }

        if (!nameModded && *isWorldLoaded == 1 && *playerNameSet == 1) {
            char16_t* name = reinterpret_cast<char16_t*>(processRamStart + PLAYER_NAME_ADDR);
            size_t nameLength = std::char_traits<char16_t>::length(name);
            if (nameLength + vc3NameLength > 16) {
                size_t offset = 16 - vc3NameLength;
                memcpy(name + offset, vc3Name, vc3NameLength * sizeof(char16_t));
            } else {
                memcpy(name + nameLength, vc3Name, (vc3NameLength * sizeof(char16_t)) + 1); // +1 for terminating null
            }

            nameModded = true;
        }

        if (*isWorldLoaded == 0 && *playerNameSet == 0) {
            if (inventoryModded)
                inventoryModded = false;

            if (nameModded)
                nameModded = false;
        }

        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(250)); // check stuff 4 times a second
    }
}

} // namespace AutomataMod