#pragma once

#include <cstdint>

namespace UI {

class PhaseJump {
    bool m_visible;

public:
    void render(uint64_t processRamStart);
};

} // namespace UI