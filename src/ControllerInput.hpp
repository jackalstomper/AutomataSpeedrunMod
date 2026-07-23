#pragma once

#include "infra/defs.hpp"
#include <array>
#include <string>

namespace AutomataMod::ControllerInput {

enum class DisplayStyle : u8 {
	Xbox,
	PlayStation,
};

struct DirectInputSnapshot {
	std::array<u8, 128> buttons{};
	std::array<u16, 6> axes{};
	u32 pov = ~0u;
};

/// Starts discovery and polling for DirectInput-compatible controllers.
void start();

/// Stops controller discovery and releases DirectInput devices.
void stop();

/// Selects Xbox or PlayStation names independently of the connected controller.
void setDisplayStyle(DisplayStyle style);
DisplayStyle getDisplayStyle();
DisplayStyle toggleDisplayStyle();

/// Starts an interactive mapping sequence for non-standard DirectInput layouts.
void startCalibration();
bool isCalibrationActive();

/// Stores the latest state for player one's XInput controller.
void updateXInput(u16 buttons, u8 leftTrigger, u8 rightTrigger);

/// Clears the XInput state when player one's controller is disconnected.
void clearXInput();

/// Stores the latest state from a DirectInput-compatible controller.
void updateDirectInput(const DirectInputSnapshot &state);

/// Clears the DirectInput state when no compatible controller is available.
void clearDirectInput();

/// Returns a compact list of the controls that are currently held.
std::wstring getPressedButtons();

} // namespace AutomataMod::ControllerInput
