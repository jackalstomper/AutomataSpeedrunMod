#include "AutomataMod.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include "Log.hpp"
#include "Util.hpp"

namespace AutomataMod {

ModChecker::ModChecker(uint64_t processRamStart, ModConfig&& modConfig) :
    m_modConfig(std::move(modConfig)),
    m_inventoryManager(processRamStart + modConfig.getAddresses().itemTableStart),
    m_chipManager(processRamStart + modConfig.getAddresses().chipTableStart),
    m_mackerelVolume(Volume(Vector3f(324.f, -100.f, 717.f), 293.f, 50.f, 253.f))
{
    m_currentPhase = reinterpret_cast<char*>(processRamStart + modConfig.getAddresses().currentPhase);
    m_playerNameSet = reinterpret_cast<uint32_t*>(processRamStart + modConfig.getAddresses().playerSetName);
    m_isWorldLoaded = reinterpret_cast<uint32_t*>(processRamStart + modConfig.getAddresses().isWorldLoaded);
    m_isLoading = reinterpret_cast<uint32_t*>(processRamStart + modConfig.getAddresses().isLoading);
    m_playerLocationPtr = reinterpret_cast<uint64_t*>(processRamStart + modConfig.getAddresses().playerLocation);
    m_unitDataFlags = reinterpret_cast<uint8_t*>(processRamStart + modConfig.getAddresses().unitData);

    m_inventoryModded = false;
    m_fishAdded = false;
    m_dvdModeEnabled = false;
    m_tauntChipsAdded = false;
}

void ModChecker::checkStuff(CComPtr<DxWrappers::DXGIFactoryWrapper> factoryWrapper)
{
    if (*m_isWorldLoaded == 1 && *m_playerNameSet == 1) {
        Vector3f* playerLocation = reinterpret_cast<Vector3f*>(m_playerLocationPtr);
        if (!m_inventoryModded && strncmp(m_currentPhase, "58_AB_BossArea_Fall", 19) == 0) {
            log(LogLevel::LOG_INFO, "Detected we are in 58_AB_BossArea_Fall. Giving VC3 inventory");
            setVc3Inventory();
            m_inventoryModded = true;
        } else if (!m_tauntChipsAdded && (m_unitDataFlags[7] & 2) && strncmp(m_currentPhase, "52_AB_Danchi_Haikyo", 19) == 0) {
            log(LogLevel::LOG_INFO, "Detected we are in 52_AB_Danchi_Haikyo and player has killed a small desert flyer. Adding Taunt+2 chips.");
            modifyChipInventory();
            m_tauntChipsAdded = true;
        } else if (!m_fishAdded && strncmp(m_currentPhase, "00_60_A_RobotM_Pro_Tutorial", 27) == 0) {
            m_fishAdded = adjustFishInventory(!m_mackerelVolume.contains(*playerLocation));
        }
    }

    if (*m_isWorldLoaded == 0 && *m_playerNameSet == 0) {
        if (m_inventoryModded || m_tauntChipsAdded || m_fishAdded) {
            log(LogLevel::LOG_INFO, "Detected the run has been reset. Resetting inventory checker.");
            log(LogLevel::LOG_INFO, "-------------------------------------------------------------------------------");
            m_inventoryModded = false;
            m_tauntChipsAdded = false;
            m_fishAdded = false;
        }
    }

    if (*m_isLoading) {
        if (!m_dvdModeEnabled) {
            factoryWrapper->toggleDvdMode(true);
            m_dvdModeEnabled = true;
        }
    } else if (m_dvdModeEnabled) {
        factoryWrapper->toggleDvdMode(false);
        m_dvdModeEnabled = false;
    }
}

bool ModChecker::adjustFishInventory(bool shouldDeleteFish)
{
    std::vector<InventoryManager::Iterator> fishies = m_inventoryManager.getAllItemsByRange(
        InventoryManager::FISH_AROWANA_ID, InventoryManager::FISH_BROKEN_FIREARM_ID);

    if (fishies.size() > 0) {
        if (shouldDeleteFish) {
            for (auto fish : fishies)
                fish->reset();
        } else {
            logEx(LogLevel::LOG_INFO, "Overriding fish with id {}", fishies[0]->itemId);
            fishies[0]->itemId = InventoryManager::FISH_MACKEREL_ID;
            log(LogLevel::LOG_INFO, "Done overwriting fish in inventory.");
            return true;
        }
    }

    return false;
}

void ModChecker::setVc3Inventory()
{
    // In order to get VC3 after adam pit we need:
    // 4 dented plates
    // 3 severed cables
    log(LogLevel::LOG_INFO, "Checking Dented Plates");
    addInventory(InventoryManager::DENTED_PLATE_ID, 4);
    log(LogLevel::LOG_INFO, "Checking Severed Cables");
    addInventory(InventoryManager::SEVERED_CABLE_ID, 3);
    log(LogLevel::LOG_INFO, "Done adjusting inventory");
}

void ModChecker::addInventory(uint32_t itemId, uint32_t quantity)
{
    InventoryManager::Iterator item = m_inventoryManager.getItemSlotById(itemId);
    if (item == m_inventoryManager.end()) {
        logEx(LogLevel::LOG_INFO, "No items found. Adding {} items", quantity);
        m_inventoryManager.addItem({ itemId, ~0u, quantity });
    } else {
        logEx(LogLevel::LOG_INFO, "Found {} items. Adjusting count to {}", item->quantity, quantity);
        item->quantity = quantity;
    }
}

void ModChecker::modifyChipInventory() {
    size_t tauntCount = 0;
    // Find existing taunt 2 chips and change their size to 6 if found
    for (auto i = m_chipManager.begin(); i != m_chipManager.end(); ++i) {
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
            m_chipManager.addChip(newTauntChip);

        logEx(LogLevel::LOG_INFO, "Added {} Taunt+2 chips.", addCount);
    } else {
        log(LogLevel::LOG_INFO, "Player already has 2 Taunt+2 chips.");
    }
}

} // namespace AutomataMod