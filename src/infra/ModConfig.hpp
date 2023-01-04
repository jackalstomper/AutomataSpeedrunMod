#pragma once

#include "infra/defs.hpp"

namespace AutomataMod {

struct Addresses {
	u64 ramStart;
	u64 currentPhase;
	u64 isWorldLoaded;
	u64 playerSetName;
	u64 isLoading;
	u64 itemTableStart;
	u64 chipTableStart;
	u64 playerLocation;
	u64 unitData;
	u64 windowMode;
};

} // namespace AutomataMod
