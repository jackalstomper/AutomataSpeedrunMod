#pragma once

#include "infra/PointerIterator.hpp"
#include "infra/defs.hpp"
#include <cstdint>

namespace AutomataMod {

namespace Chips {

extern const u32 EMPTY_SLOT_ID; // chip ID indicating an empty chip slot
extern const u32 WAUP_CHIP_ID;
extern const u32 RAUP_CHIP_ID;
extern const u32 TAUNT2_CHIP_ID;
extern const u32 MAX_SLOT_COUNT; // total amount of chips that can be stored on a player profile

struct Slot {
	u32 unknown0; // starts at 0, changes based on chip in unknown ways...
	u32 id;				// starts at 3001, increments for every chip + all their supported levels

	u32 chipIndex; // an index for each type of chip there is in the game (not including +1/2/3)
	u32 chipLevel; // The chip boost level (+0, +1, +2, etc)
	u32 slotCost;	 // Amount of slots the chip takes up in a set

	// position of the chip in the chip sets. Set to 0xFFFFFFFF to not assign to set
	u32 setAPos;
	u32 setBPos;
	u32 setCPos;

	u32 unknown1; // Always 0xFFFFFFFF (except OS chip, which is 0)
	u32 unknown2; // Always 0xFFFFFFFF (except OS chip, which is 0)
	u32 unknown3; // Always 0xFFFFFFFF (except OS chip, which is 0)
	u32 unknown4; // Always 0
};

class Manager {
	Slot *_firstChip; // The memory address that the chip table starts at

public:
	using Iterator = PointerIterator<Slot>;
	Manager();
	Manager(Slot *chipTableStart);

	// Returns a chip with the given chip ID present in inventory, or nullptr if none found
	Iterator getChipSlotById(u32 chipId);

	void addChip(const Slot &newChip);

	Iterator begin();

	Iterator end();
};

} // namespace Chips

} // namespace AutomataMod
