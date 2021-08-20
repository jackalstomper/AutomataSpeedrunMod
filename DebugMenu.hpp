#pragma once

#include <cstdint>
#include "PhaseJump.hpp"

namespace UI {

class DebugMenu {
    const uint64_t m_processRamStart;
    PhaseJump m_phaseJump;

public:
    DebugMenu();
    void render();
};

} // namespace UI