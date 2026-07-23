#include "ControllerInput.hpp"
#include <Windows.h>
#include <Xinput.h>
#include <array>
#include <iostream>

namespace {

using AutomataMod::ControllerInput::DirectInputSnapshot;

bool expect(const std::wstring &expected) {
	const std::wstring actual = AutomataMod::ControllerInput::getPressedButtons();
	if (actual == expected) {
		return true;
	}

	std::wcerr << L"Expected: " << expected << L"\nActual:   " << actual << L'\n';
	return false;
}

DirectInputSnapshot neutralState() {
	DirectInputSnapshot state;
	state.axes.fill(32768);
	return state;
}

void mapButton(u8 buttonIndex) {
	DirectInputSnapshot pressed = neutralState();
	pressed.buttons[buttonIndex] = 0x80;
	AutomataMod::ControllerInput::updateDirectInput(pressed);
	AutomataMod::ControllerInput::updateDirectInput(neutralState());
}

void mapAxis(u8 axisIndex, u16 value) {
	DirectInputSnapshot pressed = neutralState();
	pressed.axes[axisIndex] = value;
	AutomataMod::ControllerInput::updateDirectInput(pressed);
	AutomataMod::ControllerInput::updateDirectInput(neutralState());
}

} // namespace

int main() {
	using namespace AutomataMod;

	ControllerInput::clearXInput();
	ControllerInput::clearDirectInput();
	ControllerInput::setDisplayStyle(ControllerInput::DisplayStyle::Xbox);
	if (!expect(L"Buttons [Xbox]: -")) {
		return 1;
	}

	ControllerInput::updateXInput(
			XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_Y | XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_DPAD_UP,
			XINPUT_GAMEPAD_TRIGGER_THRESHOLD + 1, 0
	);
	if (!expect(L"Buttons [Xbox]: A Y LB LT D-Up")) {
		return 1;
	}

	ControllerInput::setDisplayStyle(ControllerInput::DisplayStyle::PlayStation);
	if (!expect(L"Buttons [PS]: \u00D7 \u25B3 L1 L2 D-Up")) {
		return 1;
	}

	// The default DirectInput mapping follows the standard gamepad face-button order.
	ControllerInput::clearXInput();
	DirectInputSnapshot standardState = neutralState();
	standardState.buttons[0] = 0x80;
	standardState.buttons[1] = 0x80;
	standardState.buttons[2] = 0x80;
	standardState.buttons[3] = 0x80;
	ControllerInput::updateDirectInput(standardState);
	if (!expect(L"Buttons [PS]: \u00D7 \u25CB \u25A1 \u25B3")) {
		return 1;
	}

	// Unmapped axes, including touch coordinates, must not appear as L2/R2.
	DirectInputSnapshot touchMovement = neutralState();
	touchMovement.axes[3] = 62000;
	touchMovement.axes[4] = 1000;
	ControllerInput::updateDirectInput(touchMovement);
	if (!expect(L"Buttons [PS]: -")) {
		return 1;
	}

	// Calibrate a deliberately shuffled controller and a shared bidirectional trigger axis.
	ControllerInput::startCalibration();
	ControllerInput::updateDirectInput(neutralState());
	if (!expect(L"Map [1/12]: press \u00D7")) {
		return 1;
	}

	mapButton(2);  // South / Cross
	mapButton(0);  // East / Circle
	mapButton(1);  // West / Square
	mapButton(3);  // North / Triangle
	mapButton(5);  // L1
	mapButton(4);  // R1
	mapAxis(2, 60000); // L2
	mapAxis(2, 1000);  // R2
	mapButton(9);  // Back / Share
	mapButton(8);  // Start / Options
	mapButton(11); // L3
	mapButton(10); // R3

	if (ControllerInput::isCalibrationActive()) {
		std::wcerr << L"Calibration did not finish\n";
		return 1;
	}

	DirectInputSnapshot calibratedState = neutralState();
	calibratedState.buttons[2] = 0x80;
	calibratedState.buttons[0] = 0x80;
	calibratedState.buttons[1] = 0x80;
	calibratedState.buttons[3] = 0x80;
	calibratedState.buttons[5] = 0x80;
	calibratedState.buttons[4] = 0x80;
	calibratedState.axes[2] = 60000;
	ControllerInput::updateDirectInput(calibratedState);
	if (!expect(L"Buttons [PS]: \u00D7 \u25CB \u25A1 \u25B3 L1 R1 L2")) {
		return 1;
	}

	ControllerInput::setDisplayStyle(ControllerInput::DisplayStyle::Xbox);
	if (!expect(L"Buttons [Xbox]: A B X Y LB RB LT")) {
		return 1;
	}

	// Touch-axis movement remains ignored after trigger calibration.
	touchMovement = neutralState();
	touchMovement.axes[3] = 62000;
	touchMovement.axes[4] = 1000;
	ControllerInput::updateDirectInput(touchMovement);
	if (!expect(L"Buttons [Xbox]: -")) {
		return 1;
	}

	ControllerInput::clearDirectInput();
	return expect(L"Buttons [Xbox]: -") ? 0 : 1;
}
