#include "ChipManager.hpp"

#include <type_traits>

namespace AutomataMod {

namespace Chips {

const u32 MAX_SLOT_COUNT = 300; // total amount of chips that can be stored on a player profile
const u32 EMPTY_SLOT_ID = ~0;		// chip ID indicating an empty chip slot
const u32 WAUP_CHIP_ID = 3007;
const u32 RAUP_CHIP_ID = 3034;
const u32 TAUNT2_CHIP_ID = 3228;

Manager::Manager(Slot *chipTableStart) : _firstChip(chipTableStart) {}

// Returns a chip with the given chip ID present in inventory, or nullptr if none found
Manager::Iterator Manager::getChipSlotById(u32 chipId) {
	for (auto i = begin(); i != end(); ++i) {
		if (i->id == chipId)
			return i;
	}

	return end();
}

void Manager::addChip(const Slot &newChip) {
	Iterator slot = getChipSlotById(EMPTY_SLOT_ID);
	if (slot != end())
		*slot = newChip;
}

Manager::Iterator Manager::begin() { return Iterator(_firstChip); }

Manager::Iterator Manager::end() { return Iterator(_firstChip + MAX_SLOT_COUNT); }

static_assert(std::is_trivially_copyable<Slot>::value, "ChipSlot must be POD");
static_assert(sizeof(Slot) == 48,
							"ChipSlot isn't 48 bytes in size! This breaks pointer reading logic with game memory!");

} // namespace Chips

} // namespace AutomataMod
