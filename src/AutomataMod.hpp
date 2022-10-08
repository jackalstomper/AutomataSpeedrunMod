#pragma once

#include <cstdint>
#include <wrl/client.h>

#include "ChipManager.hpp"
#include "InventoryManager.hpp"
#include "com/FactoryWrapper.hpp"
#include "infra/ModConfig.hpp"
#include "infra/Util.hpp"

namespace AutomataMod {

class ModChecker {
	ModConfig m_modConfig;
	InventoryManager m_inventoryManager;
	ChipManager m_chipManager;
	Volume m_mackerelVolume;

	char *m_currentPhase;
	uint32_t *m_playerNameSet;
	uint64_t *m_playerLocationPtr;
	uint32_t *m_isWorldLoaded;
	uint32_t *m_isLoading;

	// Unit data is a collection of 8 bit bitmasks that indicate if a player has killed a unit or not.
	// Use these flags to determine if player has killed a small flyer in the correct phase to give taunt chips
	// buffer size: 24 bytes
	uint8_t *m_unitDataFlags;

	bool m_inventoryModded;
	bool m_fishAdded;
	bool m_dvdModeEnabled;
	bool m_tauntChipsAdded;

	void modifyChipInventory();

	void addInventory(uint32_t itemId, uint32_t quantity);

	void setVc3Inventory();

	bool adjustFishInventory(bool shouldDeleteFish);

public:
	ModChecker(uint64_t processRamStart, ModConfig &&modConfig);
	void checkStuff(Microsoft::WRL::ComPtr<DxWrappers::DXGIFactoryWrapper> factoryWrapper);
};

} // namespace AutomataMod
