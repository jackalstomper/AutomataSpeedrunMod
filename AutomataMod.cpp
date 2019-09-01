#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include "InventoryManager.hpp"
#include "Log.hpp"
#include "DxWrappers.hpp"

extern DxWrappers::DXGIFactoryWrapper* g_wrapper;

namespace {

// Addresses are offsets in bytes relevant to processRamStart
const uint64_t CURRENT_PHASE_ADDR = 0x1101D58;
const uint64_t PLAYER_SET_NAME_ADDR = 0x147B4BC;
const uint64_t IS_WORLD_LOADED_ADDR = 0x110ADC0;
const uint64_t ITEM_TABLE_START_ADDR = 0x197C4C4;
const uint64_t IS_LOADING_ADDR = 0x147BF50;

char* currentPhase = nullptr;
char16_t* playerName = nullptr;
uint32_t* playerNameSet = nullptr;
uint32_t* isWorldLoaded = nullptr;
uint32_t* isLoading = nullptr;
bool inventoryModded = false;
bool dvdModeEnabled = false;

} // namespace

namespace AutomataMod {

void checkStuff(uint64_t processRamStart)
{
    AutomataMod::InventoryManager inventoryManager(processRamStart + ITEM_TABLE_START_ADDR);
    currentPhase = reinterpret_cast<char*>(processRamStart + CURRENT_PHASE_ADDR);
    playerNameSet = reinterpret_cast<uint32_t*>(processRamStart + PLAYER_SET_NAME_ADDR);
    isWorldLoaded = reinterpret_cast<uint32_t*>(processRamStart + IS_WORLD_LOADED_ADDR);
    isLoading = reinterpret_cast<uint32_t*>(processRamStart + IS_LOADING_ADDR);

    AutomataMod::log(LogLevel::LOG_INFO, "Checker thread started. Waiting for change conditions");

    while (true) {
        if (!inventoryModded && *isWorldLoaded == 1 && *playerNameSet == 1 && strncmp(currentPhase, "58_AB_BossArea_Fall", 19) == 0) {
            AutomataMod::log(LogLevel::LOG_INFO, "Detected we are in 58_AB_BossArea_Fall. Giving VC3 inventory");
            inventoryManager.setVc3Inventory();
            inventoryModded = true;
        }

        if (*isWorldLoaded == 0 && *playerNameSet == 0) {
            if (inventoryModded) {
                AutomataMod::log(LogLevel::LOG_INFO, "Detected the run has been reset. Resetting inventory checker.");
                AutomataMod::log(LogLevel::LOG_INFO, "-------------------------------------------------------------------------------");
                inventoryModded = false;
            }
        }

        if (*isLoading && g_wrapper) {
            if (!dvdModeEnabled) {
                g_wrapper->toggleDvdMode(true);
                dvdModeEnabled = true;
            }
        } else if (dvdModeEnabled && g_wrapper) {
            g_wrapper->toggleDvdMode(false);
            dvdModeEnabled = false;
        }

        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(250)); // check stuff 4 times a second
    }
}

} // namespace AutomataMod