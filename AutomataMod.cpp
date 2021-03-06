﻿#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include "InventoryManager.hpp"
#include "ChipManager.hpp"
#include "Log.hpp"
#include "DxWrappers.hpp"
#include "Util.hpp"

extern DxWrappers::DXGIFactoryWrapper* g_wrapper;

namespace {

using namespace AutomataMod;

// Addresses are offsets in bytes relevant to processRamStart
const uint64_t CURRENT_PHASE_ADDR = 0x1101D58;
const uint64_t PLAYER_SET_NAME_ADDR = 0x147B4BC;
const uint64_t IS_WORLD_LOADED_ADDR = 0x110ADC0;
const uint64_t ITEM_TABLE_START_ADDR = 0x197C4C4;
const uint64_t IS_LOADING_ADDR = 0x147BF50;
const uint64_t PLAYER_LOCATION_PTR_ADDR = 0x16053E8;
const uint64_t CHIP_TABLE_START_ADDR = 0x197E410;
const uint64_t UNIT_DATA_START_ADDR = 0x19844C8;

char* currentPhase = nullptr;
char16_t* playerName = nullptr;
uint32_t* playerNameSet = nullptr;
uint64_t* playerLocationPtr = nullptr;
uint32_t* isWorldLoaded = nullptr;
uint32_t* isLoading = nullptr;
bool inventoryModded = false;
bool fishAdded = false;
bool dvdModeEnabled = false;
bool tauntChipsAdded = false;

Volume mackerelVolume(Vector3f(324.f, -100.f, 717.f), 293.f, 50.f, 253.f);

void modifyChipInventory(ChipManager& chipManager) {
    size_t tauntCount = 0;
    // Find existing taunt 2 chips and change their size to 6 if found
    for (auto i = chipManager.begin(); i != chipManager.end(); ++i) {
        if (i->id == ChipManager::TAUNT2_CHIP_ID) {
            log(LogLevel::LOG_INFO, "Found Taunt+2 chip. Setting size to 6.");
            ++tauntCount;
            i->slotCost = 6;
        }

        if (tauntCount >= 2)
            break;
    }

    // Ensure we have a minimum of 2 T+2 chips
    if (tauntCount < 2) {
        size_t addCount = 2 - tauntCount;
        ChipManager::ChipSlot newTauntChip{ 228u, 3228u, 25u, 2u, 6u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 0u };
        for (size_t i = 0; i < addCount; ++i)
            chipManager.addChip(newTauntChip);

        log(LogLevel::LOG_INFO, "Added " + std::to_string(addCount) + " Taunt+2 chips.");
    } else {
        log(LogLevel::LOG_INFO, "Player already has 2 Taunt+2 chips.");
    }
}

void addInventory(InventoryManager& inventoryManager, uint32_t itemId, uint32_t quantity)
{
    InventoryManager::Iterator item = inventoryManager.getItemSlotById(itemId);
    if (item == inventoryManager.end()) {
        log(LogLevel::LOG_INFO, "No items found. Adding " + std::to_string(quantity) + " items");
        inventoryManager.addItem({ itemId, ~0u, quantity });
    } else {
        log(LogLevel::LOG_INFO, "Found " + std::to_string(item->quantity) + " items. Adjusting count to " + std::to_string(quantity));
        item->quantity = quantity;
    }
}

void setVc3Inventory(InventoryManager& inventoryManager)
{
    // In order to get VC3 after adam pit we need:
    // 4 dented plates
    // 3 severed cables
    log(LogLevel::LOG_INFO, "Checking Dented Plates");
    addInventory(inventoryManager, InventoryManager::DENTED_PLATE_ID, 4);
    log(LogLevel::LOG_INFO, "Checking Severed Cables");
    addInventory(inventoryManager, InventoryManager::SEVERED_CABLE_ID, 3);
    log(LogLevel::LOG_INFO, "Done adjusting inventory");
}

bool adjustFishInventory(InventoryManager& inventoryManager, bool shouldDeleteFish)
{
    std::vector<InventoryManager::Iterator> fishies = inventoryManager.getAllItemsByRange(
        InventoryManager::FISH_AROWANA_ID, InventoryManager::FISH_BROKEN_FIREARM_ID);

    if (fishies.size() > 0) {
        if (shouldDeleteFish) {
            for (auto fish : fishies)
                fish->reset();
        } else {
            log(LogLevel::LOG_INFO, "Overriding fish with id " + std::to_string(fishies[0]->itemId));
            fishies[0]->itemId = InventoryManager::FISH_MACKEREL_ID;
            log(LogLevel::LOG_INFO, "Done overwriting fish in inventory.");
            return true;
        }
    }

    return false;
}

} // namespace

namespace AutomataMod {

void checkStuff(uint64_t processRamStart)
{
    InventoryManager inventoryManager(processRamStart + ITEM_TABLE_START_ADDR);
    ChipManager chipManager(processRamStart + CHIP_TABLE_START_ADDR);
    currentPhase = reinterpret_cast<char*>(processRamStart + CURRENT_PHASE_ADDR);
    playerNameSet = reinterpret_cast<uint32_t*>(processRamStart + PLAYER_SET_NAME_ADDR);
    isWorldLoaded = reinterpret_cast<uint32_t*>(processRamStart + IS_WORLD_LOADED_ADDR);
    isLoading = reinterpret_cast<uint32_t*>(processRamStart + IS_LOADING_ADDR);
    playerLocationPtr = reinterpret_cast<uint64_t*>(processRamStart + PLAYER_LOCATION_PTR_ADDR);

    // Unit data is a collection of 8 bit bitmasks that indicate if a player has killed a unit or not.
    // Use these flags to determine if player has killed a small flyer in the correct phase to give taunt chips
    // buffer size: 24 bytes
    const uint8_t* unitDataFlags = reinterpret_cast<uint8_t*>(processRamStart + UNIT_DATA_START_ADDR);

    log(LogLevel::LOG_INFO, "Checker thread started. Waiting for change conditions");

    while (true) {
        if (*isWorldLoaded == 1 && *playerNameSet == 1) {
            Vector3f* playerLocation = reinterpret_cast<Vector3f*>(*playerLocationPtr + 0x50);
            if (!inventoryModded && strncmp(currentPhase, "58_AB_BossArea_Fall", 19) == 0) {
                log(LogLevel::LOG_INFO, "Detected we are in 58_AB_BossArea_Fall. Giving VC3 inventory");
                setVc3Inventory(inventoryManager);
                inventoryModded = true;
            } else if (!tauntChipsAdded && (unitDataFlags[7] & 2) && strncmp(currentPhase, "52_AB_Danchi_Haikyo", 19) == 0) {
                log(LogLevel::LOG_INFO, "Detected we are in 52_AB_Danchi_Haikyo and player has killed a small desert flyer. Adding Taunt+2 chips.");
                modifyChipInventory(chipManager);
                tauntChipsAdded = true;
            } else if (!fishAdded && strncmp(currentPhase, "00_60_A_RobotM_Pro_Tutorial", 27) == 0) {
                fishAdded = adjustFishInventory(inventoryManager, !mackerelVolume.contains(*playerLocation));
            }
        }

        if (*isWorldLoaded == 0 && *playerNameSet == 0) {
            if (inventoryModded || tauntChipsAdded || fishAdded) {
                log(LogLevel::LOG_INFO, "Detected the run has been reset. Resetting inventory checker.");
                log(LogLevel::LOG_INFO, "-------------------------------------------------------------------------------");
                inventoryModded = false;
                tauntChipsAdded = false;
                fishAdded = false;
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