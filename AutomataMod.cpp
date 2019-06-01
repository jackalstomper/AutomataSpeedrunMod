#include <cstdint>
#include <cstring>
#include <string>
#include "InventoryManager.hpp"
#include "Log.hpp"

namespace {

const char16_t* VC3_NAME = u"vc3v1.2";
const size_t VC3_NAME_LEN = std::char_traits<char16_t>::length(VC3_NAME);

// Addresses are offsets in bytes relevant to processRamStart
const uint64_t CURRENT_PHASE_ADDR = 0x1101D58;
const uint64_t PLAYER_SET_NAME_ADDR = 0x147B4BC;
const uint64_t IS_WORLD_LOADED_ADDR = 0x110ADC0;
const uint64_t ITEM_TABLE_START_ADDR = 0x197C4C4;
const uint64_t PLAYER_NAME_ADDR = 0x194BF88;

char* currentPhase = nullptr;
char16_t* playerName = nullptr;
uint32_t* playerNameSet = nullptr;
uint32_t* isWorldLoaded = nullptr;
bool inventoryModded = false;
bool nameModded = false;

// number of frames since our last condition check
uint32_t counter = 0;

} // namespace

namespace AutomataMod {

void checkStuff(uint64_t processRamStart)
{
    static AutomataMod::InventoryManager inventoryManager(processRamStart + ITEM_TABLE_START_ADDR);

    ++counter;
    if (counter < 15) // Only check conditions every 15 frames
        return;

    counter = 0;

    if (!currentPhase) {
        currentPhase = reinterpret_cast<char*>(processRamStart + CURRENT_PHASE_ADDR);
        playerName = reinterpret_cast<char16_t*>(processRamStart + PLAYER_NAME_ADDR);
        playerNameSet = reinterpret_cast<uint32_t*>(processRamStart + PLAYER_SET_NAME_ADDR);
        isWorldLoaded = reinterpret_cast<uint32_t*>(processRamStart + IS_WORLD_LOADED_ADDR);
    }

    if (!inventoryModded && *isWorldLoaded == 1 && *playerNameSet == 1 && strncmp(currentPhase, "58_AB_BossArea_Fall", 19) == 0) {
        AutomataMod::log(LogLevel::LOG_INFO, "Detected we are in 58_AB_BossArea_Fall. Giving VC3 inventory");
        inventoryManager.setVc3Inventory();
        inventoryModded = true;
    }

    if (!nameModded && *isWorldLoaded == 1 && *playerNameSet == 1) {
        AutomataMod::log(LogLevel::LOG_INFO, "Detected we need to change savefile name. Changing.");
        size_t nameLen = std::char_traits<char16_t>::length(playerName);
        if (nameLen + VC3_NAME_LEN > 16) {
            size_t offset = 16 - VC3_NAME_LEN;
            memcpy(playerName + offset, VC3_NAME, VC3_NAME_LEN * sizeof(char16_t));
        } else {
            memcpy(playerName + nameLen, VC3_NAME, (VC3_NAME_LEN * sizeof(char16_t)) + 1); // +1 for terminating null
        }

        nameModded = true;
    }

    if (*isWorldLoaded == 0 && *playerNameSet == 0) {
        if (inventoryModded) {
            AutomataMod::log(LogLevel::LOG_INFO, "Detected the run has been reset. Resetting inventory checker.");
            AutomataMod::log(LogLevel::LOG_INFO, "-------------------------------------------------------------------------------");
            inventoryModded = false;
        }

        if (nameModded)
            nameModded = false;
    }
}

} // namespace AutomataMod