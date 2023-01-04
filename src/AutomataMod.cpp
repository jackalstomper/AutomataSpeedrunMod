#include "AutomataMod.hpp"
#include "infra/Log.hpp"

namespace {

// Unit data is a collection of 8 bit bitmasks that indicate if a player has killed a unit or not.
// Use these flags to determine if player has killed a small flyer in the correct phase to give taunt chips
// buffer size: 24 bytes
const size_t UNIT_DATA_SIZE = 24;

} // namespace

namespace AutomataMod {

bool ModChecker::inPhase(const char *phase) { return strncmp(_currentPhase, phase, strlen(phase)) == 0; }

void ModChecker::modifyChipInventory() {
	size_t tauntCount = 0;
	// Find existing taunt 2 chips and change their size to 6 if found
	for (auto i = _chipManager.begin(); i != _chipManager.end(); ++i) {
		if (i->id == Chips::TAUNT2_CHIP_ID) {
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
		Chips::Slot newTauntChip{228u, 3228u, 25u, 2u, 6u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 0u};
		for (size_t i = 0; i < addCount; ++i)
			_chipManager.addChip(newTauntChip);

		log(LogLevel::LOG_INFO, "Added {} Taunt+2 chips.", addCount);
	} else {
		log(LogLevel::LOG_INFO, "Player already has 2 Taunt+2 chips.");
	}
}

void ModChecker::addInventory(u32 itemId, u32 quantity) {
	Inventory::Manager::Iterator item = _inventoryManager.getItemSlotById(itemId);
	if (item == _inventoryManager.end()) {
		log(LogLevel::LOG_INFO, "No items found. Adding {} items", quantity);
		_inventoryManager.addItem({itemId, ~0u, quantity});
	} else {
		log(LogLevel::LOG_INFO, "Found {} items. Adjusting count to {}", item->quantity, quantity);
		item->quantity = quantity;
	}
}

void ModChecker::setVc3Inventory() {
	// In order to get VC3 after adam pit we need:
	// 4 dented plates
	// 3 severed cables
	log(LogLevel::LOG_INFO, "Checking Dented Plates");
	addInventory(Inventory::DENTED_PLATE_ID, 4);
	log(LogLevel::LOG_INFO, "Checking Severed Cables");
	addInventory(Inventory::SEVERED_CABLE_ID, 3);
	log(LogLevel::LOG_INFO, "Done adjusting inventory");
}

bool ModChecker::adjustFishInventory(bool shouldDeleteFish) {
	std::vector<Inventory::Manager::Iterator> fishies =
			_inventoryManager.getAllItemsByRange(Inventory::FISH_AROWANA_ID, Inventory::FISH_BROKEN_FIREARM_ID);

	if (fishies.size() > 0) {
		if (shouldDeleteFish) {
			for (auto fish : fishies)
				fish->reset();
		} else {
			log(LogLevel::LOG_INFO, "Overriding fish with id {}", fishies[0]->itemId);
			fishies[0]->itemId = Inventory::FISH_MACKEREL_ID;
			log(LogLevel::LOG_INFO, "Done overwriting fish in inventory.");
			return true;
		}
	}

	return false;
}

ModChecker::ModChecker(Addresses addrs)
		: _addresses(addrs)
		, _inventoryManager(getOffset<Inventory::Item>(_addresses.itemTableStart))
		, _chipManager((getOffset<Chips::Slot>(_addresses.chipTableStart)))
		, _mackerelVolume(Vector3f(324.f, -100.f, 717.f), 293.f, 50.f, 253.f)
		, _inventoryModded(false)
		, _fishAdded(false)
		, _dvdModeEnabled(false)
		, _tauntChipsAdded(false)
		, _worldLoaded(getOffset<u32>(_addresses.isWorldLoaded))
		, _playerNameSet(getOffset<u32>(_addresses.playerSetName))
		, _currentPhase(getOffset<char>(_addresses.currentPhase))
		, _unitData(getOffset<u8>(_addresses.unitData))
		, _isLoading(getOffset<bool>(_addresses.isLoading))
		, _windowMode(getOffset<u32>(_addresses.windowMode))
		, _lastWindowMode(0)
		, _modActive(true)
		, _lastModActive(true)
		, _inMenu(true)
		, _lastInMenu(true) {}

void ModChecker::checkStuff(Microsoft::WRL::ComPtr<DxWrappers::DXGIFactoryWrapper> factoryWrapper) {
	if (_lastWindowMode != *_windowMode) {
		factoryWrapper->setWindowMode(*_windowMode);
		_lastWindowMode = *_windowMode;
	}

	if (_lastModActive != _modActive) {
		factoryWrapper->setModActive(_modActive);
		_lastModActive = _modActive;
	}

	if (_lastInMenu != _inMenu) {
		factoryWrapper->setInMenu(_inMenu);
		_lastInMenu = _inMenu;
	}

	if (*_worldLoaded == 1 && *_playerNameSet == 1) {
		_inMenu = false;
		if (_modActive) {
			if (!_inventoryModded && inPhase("58_AB_BossArea_Fall")) {
				log(LogLevel::LOG_INFO, "Detected we are in 58_AB_BossArea_Fall. Giving VC3 inventory");
				setVc3Inventory();
				_inventoryModded = true;
			} else if (!_tauntChipsAdded && (_unitData[7] & 2) && inPhase("52_AB_Danchi_Haikyo")) {
				log(LogLevel::LOG_INFO,
						"Detected we are in 52_AB_Danchi_Haikyo and player has killed a small desert flyer. Adding Taunt+2 chips.");
				modifyChipInventory();
				_tauntChipsAdded = true;
			} else if (!_fishAdded && inPhase("00_60_A_RobotM_Pro_Tutorial")) {
				Vector3f *playerLocation = getOffset<Vector3f>(_addresses.playerLocation);
				_fishAdded = adjustFishInventory(!_mackerelVolume.contains(*playerLocation));
			}
		}
	}

	if (*_worldLoaded == 0 && *_playerNameSet == 0) {
		_inMenu = true;
		if (_inventoryModded || _tauntChipsAdded || _fishAdded) {
			log(LogLevel::LOG_INFO, "Detected the run has been reset. Resetting inventory checker.");
			log(LogLevel::LOG_INFO, "-------------------------------------------------------------------------------");
			_inventoryModded = false;
			_tauntChipsAdded = false;
			_fishAdded = false;
		}
	}

	if (*_isLoading) {
		if (!_dvdModeEnabled) {
			factoryWrapper->toggleDvdMode(true);
			_dvdModeEnabled = true;
		}
	} else if (_dvdModeEnabled) {
		factoryWrapper->toggleDvdMode(false);
		_dvdModeEnabled = false;
	}
}

bool ModChecker::validCheckState() { return *_worldLoaded == 1 && *_playerNameSet == 1; }

bool ModChecker::getModActive() const { return _modActive; }

void ModChecker::setModActive(bool active) {
	if (!active) {
		log(LogLevel::LOG_INFO, "Mod disabled by user input.");
	} else {
		log(LogLevel::LOG_INFO, "Mod enabled by user input.");
	}
	_modActive = active;
}

} // namespace AutomataMod
