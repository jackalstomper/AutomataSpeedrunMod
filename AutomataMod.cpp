#include <cstdint>
#include <cstring>
#include <chrono>
#include <thread>
#include "InventoryManager.hpp"

namespace {

// Addresses are offsets in bytes relevant to processRamStart
const uint64_t CURRENT_PHASE_ADDR = 0x1101D58;
const uint64_t PLAYER_SET_NAME_ADDR = 0x147B4BC;
const uint64_t PLAYER_NAME_ADDR = 0x194BF88;
const uint64_t IS_WORLD_LOADED_ADDR = 0x110ADC0;
const uint64_t ITEM_TABLE_START_ADDR = 0x197C4C4;

const char16_t* MOD_NAME_POSTFIX = u"|AM";
const int MOD_NAME_POSTFIX_LEN = 3;


// Our offsets are in bytes, so we need to compute the pointer using a byte sized type before converting into the type we desire
template<typename T>
T getOffsetPointer(uint8_t* processRamStart, uint64_t address)
{
    return reinterpret_cast<T>(processRamStart + address);
}

// Adds a signature onto the players savegame name to indicate the mod is in use
bool addPostfixToFilename(char16_t* playerName)
{
    int nameLen = 0;
    for (char16_t* i = playerName; i < playerName + 16; ++i, ++nameLen) {
        if (*i == 0)
            break;
    }

    if (nameLen == 0) // Something has gone wrong
        return false;

    char16_t* insertPoint;
    if (nameLen + MOD_NAME_POSTFIX_LEN > 16) {
        // Need to trim the name to fit our postfix
        insertPoint = playerName + nameLen - 3;
    } else {
        insertPoint = playerName + nameLen;
    }

    memcpy(insertPoint, MOD_NAME_POSTFIX, MOD_NAME_POSTFIX_LEN * sizeof(char16_t));
    return true;
}

} // namespace

namespace AutomataMod {

void checkStuff(uint8_t* processRamStart)
{
    char* currentPhase = getOffsetPointer<char*>(processRamStart, CURRENT_PHASE_ADDR);
    uint32_t* playerNameSet = getOffsetPointer<uint32_t*>(processRamStart, PLAYER_SET_NAME_ADDR);
    char16_t* playerName = getOffsetPointer<char16_t*>(processRamStart, PLAYER_NAME_ADDR);
    uint32_t* isWorldLoaded = getOffsetPointer<uint32_t*>(processRamStart, IS_WORLD_LOADED_ADDR);
    uint32_t* itemTableRamStart = getOffsetPointer<uint32_t*>(processRamStart, ITEM_TABLE_START_ADDR);
    bool modNameAdded = false;
    bool inventoryModded = false;

    while (true) {
        if (!modNameAdded && *isWorldLoaded == 1 && *playerNameSet == 1) {
            modNameAdded = addPostfixToFilename(playerName);
        }

        if (!inventoryModded && *isWorldLoaded == 1 && *playerNameSet == 1 && strncmp(currentPhase, "58_AB_BossArea_Fall", 19) == 0) {
            AutomataMod::InventoryManager manager(itemTableRamStart);
            manager.setVc3Inventory();
            inventoryModded = true;
        }

        if (*isWorldLoaded == 0 && *playerNameSet == 0) {
            if (modNameAdded)
                modNameAdded = false;

            if (inventoryModded)
                inventoryModded = false;
        }

        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(250)); // check stuff 4 times a second
    }
}

} // namespace AutomataMod