#include "ChipManager.hpp"

#include <type_traits>

namespace AutomataMod {

using Iterator = ChipManager::Iterator;

const int ChipManager::MAX_SLOT_COUNT = 300;		// total amount of chips that can be stored on a player profile
const uint32_t ChipManager::EMPTY_SLOT_ID = ~0; // chip ID indicating an empty chip slot
const uint32_t ChipManager::WAUP_CHIP_ID = 3007;
const uint32_t ChipManager::RAUP_CHIP_ID = 3034;
const uint32_t ChipManager::TAUNT2_CHIP_ID = 3228;

ChipManager::ChipManager(uint64_t chipTableRamStart) : _firstChip(reinterpret_cast<ChipSlot *>(chipTableRamStart)) {}

// Returns a chip with the given chip ID present in inventory, or nullptr if none found
Iterator ChipManager::getChipSlotById(uint32_t chipId) {
	for (auto i = begin(); i != end(); ++i) {
		if (i->id == chipId)
			return i;
	}

	return end();
}

void ChipManager::addChip(const ChipSlot &newChip) {
	Iterator slot = getChipSlotById(EMPTY_SLOT_ID);
	if (slot != end())
		*slot = newChip;
}

Iterator ChipManager::begin() { return Iterator(_firstChip); }

Iterator ChipManager::end() { return Iterator(_firstChip + MAX_SLOT_COUNT); }

static_assert(std::is_trivially_copyable<ChipSlot>::value, "ChipSlot must be POD");
static_assert(sizeof(ChipSlot) == 48,
							"ChipSlot isn't 48 bytes in size! This breaks pointer reading logic with game memory!");

} // namespace AutomataMod
