#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <wrl/client.h>

#include "ChipManager.hpp"
#include "InventoryManager.hpp"
#include "StickState.hpp"
#include "com/FactoryWrapper2.hpp"
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
	u32 *_windowMode;
	bool _modActive;
	StickState *_stickState;

	/// @brief Checks if game is in the given phase
	/// @param str The phase to check
	/// @return true if game is set in the given phase
	bool inPhase(const char *phase) const;

	void modifyChipInventory();
	void addInventory(u32 itemId, u32 quantity);
	void setVc3Inventory();
	bool adjustFishInventory(bool shouldDeleteFish);
	void setPersistentActiveState(bool state, bool initializing);

	template <typename T> T *getOffset(u64 offset) { return reinterpret_cast<T *>(_addresses.ramStart + offset); }

public:
	static void set(nullptr_t);
	static void set(std::unique_ptr<ModChecker> &&ptr);
	static ModChecker *get();
	ModChecker(Addresses addrs);
	void checkStuff(Microsoft::WRL::ComPtr<DxWrappers::DXGIFactoryWrapper2> factoryWrapper);

	// Returns true if the game is in a state for modding inventory
	bool validCheckState();

	bool getPersistentActiveState();
	bool getModActive() const;
	void setModActive(bool active, bool userInput);

	u32 getWindowMode() const;
	bool getInMenu() const;
};

} // namespace AutomataMod
