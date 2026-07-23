#pragma once

#include "defs.hpp"
#include <cstddef>

namespace Persistence {

void setEnabledFlag(bool enabled);
bool getEnabledFlag();

void setPlayStationButtonDisplay(bool enabled);
bool getPlayStationButtonDisplay();

void setControllerMapping(const u32 *mapping, size_t count);
bool getControllerMapping(u32 *mapping, size_t count);

} // namespace Persistence
