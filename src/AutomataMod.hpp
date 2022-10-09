#pragma once

#include <cstdint>
#include <wrl/client.h>

#include "ChipManager.hpp"
#include "InventoryManager.hpp"
#include "com/FactoryWrapper.hpp"
#include "infra/ModConfig.hpp"
#include "infra/Util.hpp"
#include "infra/defs.hpp"

namespace AutomataMod {

class ModChecker {
	Addresses _addresses;
	Inventory::Manager _inventoryManager;
	Chips::Manager _chipManager;
	Volume _mackerelVolume;

	bool _inventoryModded;
	bool _fishAdded;
	bool _dvdModeEnabled;
	bool _tauntChipsAdded;

	u32 *_worldLoaded;
	u32 *_playerNameSet;
	char *_currentPhase;
	u8 *_unitData;
	bool *_isLoading;

	/// @brief Checks if game is in the given phase
	/// @param str The phase to check
	/// @return true if game is set in the given phase
	bool inPhase(const char *phase);

	void modifyChipInventory();
	void addInventory(u32 itemId, u32 quantity);
	void setVc3Inventory();
	bool adjustFishInventory(bool shouldDeleteFish);

	template <typename T> T *getOffset(u64 offset) { return reinterpret_cast<T *>(_addresses.ramStart + offset); }

public:
	ModChecker(Addresses addrs);
	void checkStuff(Microsoft::WRL::ComPtr<DxWrappers::DXGIFactoryWrapper> factoryWrapper);
};

} // namespace AutomataMod
