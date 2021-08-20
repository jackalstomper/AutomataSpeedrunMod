#include "DebugMenu.hpp"
#include "imgui.h"
#include <Windows.h>

namespace UI {

DebugMenu::DebugMenu()
    : m_processRamStart(reinterpret_cast<uint64_t>(GetModuleHandle(nullptr)))
{}

void DebugMenu::render() {
    if (!ImGui::Begin("Debug"))
        return;

    m_phaseJump.render(m_processRamStart);
    ImGui::End();
}

} // namespace UI