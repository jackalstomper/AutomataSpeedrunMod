#include "ControllerInput.hpp"
#ifndef CONTROLLER_INPUT_TESTING
#include "infra/Persistence.hpp"
#endif
#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <Xinput.h>
#include <dinput.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace AutomataMod;
using Microsoft::WRL::ComPtr;

constexpr u32 LEFT_TRIGGER_SHIFT = 16;
constexpr u32 RIGHT_TRIGGER_SHIFT = 24;
constexpr u16 AXIS_INPUT_THRESHOLD = 4096;
constexpr auto DEVICE_RESCAN_INTERVAL = std::chrono::seconds(3);
constexpr auto POLL_INTERVAL = std::chrono::milliseconds(8);

enum class LogicalInput : size_t {
	South,
	East,
	West,
	North,
	LeftBumper,
	RightBumper,
	LeftTrigger,
	RightTrigger,
	Back,
	Start,
	LeftStick,
	RightStick,
	Count,
};

constexpr size_t LOGICAL_INPUT_COUNT = static_cast<size_t>(LogicalInput::Count);

enum class BindingType : u8 {
	Button = 1,
	AxisPositive = 2,
	AxisNegative = 3,
};

struct InputBinding {
	BindingType type;
	u8 index;
	u16 baseline;
};

struct InputLabel {
	const wchar_t *xbox;
	const wchar_t *playStation;
};

constexpr std::array<InputLabel, LOGICAL_INPUT_COUNT> INPUT_LABELS{{
		{L"A", L"\u00D7"},
		{L"B", L"\u25CB"},
		{L"X", L"\u25A1"},
		{L"Y", L"\u25B3"},
		{L"LB", L"L1"},
		{L"RB", L"R1"},
		{L"LT", L"L2"},
		{L"RT", L"R2"},
		{L"Back", L"Share"},
		{L"Start", L"Options"},
		{L"L3", L"L3"},
		{L"R3", L"R3"},
}};

constexpr std::array<InputBinding, LOGICAL_INPUT_COUNT> DEFAULT_BINDINGS{{
		{BindingType::Button, 0, 0},
		{BindingType::Button, 1, 0},
		{BindingType::Button, 2, 0},
		{BindingType::Button, 3, 0},
		{BindingType::Button, 4, 0},
		{BindingType::Button, 5, 0},
		{BindingType::Button, 6, 0},
		{BindingType::Button, 7, 0},
		{BindingType::Button, 8, 0},
		{BindingType::Button, 9, 0},
		{BindingType::Button, 10, 0},
		{BindingType::Button, 11, 0},
}};

struct XInputButtonLabel {
	u16 mask;
	LogicalInput input;
};

constexpr std::array<XInputButtonLabel, 10> XINPUT_BUTTONS{{
		{XINPUT_GAMEPAD_A, LogicalInput::South},
		{XINPUT_GAMEPAD_B, LogicalInput::East},
		{XINPUT_GAMEPAD_X, LogicalInput::West},
		{XINPUT_GAMEPAD_Y, LogicalInput::North},
		{XINPUT_GAMEPAD_LEFT_SHOULDER, LogicalInput::LeftBumper},
		{XINPUT_GAMEPAD_RIGHT_SHOULDER, LogicalInput::RightBumper},
		{XINPUT_GAMEPAD_BACK, LogicalInput::Back},
		{XINPUT_GAMEPAD_START, LogicalInput::Start},
		{XINPUT_GAMEPAD_LEFT_THUMB, LogicalInput::LeftStick},
		{XINPUT_GAMEPAD_RIGHT_THUMB, LogicalInput::RightStick},
}};

struct CalibrationState {
	bool active = false;
	bool baselineCaptured = false;
	bool waitingForRelease = true;
	size_t step = 0;
	std::array<u16, 6> baseline{};
	std::array<InputBinding, LOGICAL_INPUT_COUNT> workingBindings = DEFAULT_BINDINGS;
};

std::atomic<u32> xinputState{0};
std::atomic<bool> xinputConnected{false};
std::atomic<bool> pollerRunning{false};
std::atomic<ControllerInput::DisplayStyle> displayStyle{ControllerInput::DisplayStyle::Xbox};
std::thread pollerThread;

std::mutex directInputMutex;
ControllerInput::DirectInputSnapshot directInputState;
bool directInputConnected = false;
std::array<InputBinding, LOGICAL_INPUT_COUNT> inputBindings = DEFAULT_BINDINGS;
CalibrationState calibration;

struct DirectInputDevice {
	ComPtr<IDirectInputDevice8W> device;
};

struct DeviceEnumerationContext {
	IDirectInput8W *directInput;
	HWND window;
	std::vector<DirectInputDevice> *devices;
};

const wchar_t *getLabel(LogicalInput input, ControllerInput::DisplayStyle style) {
	const InputLabel &labels = INPUT_LABELS[static_cast<size_t>(input)];
	return style == ControllerInput::DisplayStyle::Xbox ? labels.xbox : labels.playStation;
}

std::wstring getPrefix(ControllerInput::DisplayStyle style) {
	return style == ControllerInput::DisplayStyle::Xbox ? L"Buttons [Xbox]:" : L"Buttons [PS]:";
}

void appendButton(std::wstring &result, const std::wstring &label) {
	result += L' ';
	result += label;
}

std::wstring formatXInputButtons(u32 packedState, ControllerInput::DisplayStyle style) {
	const u16 buttons = static_cast<u16>(packedState);
	const u8 leftTrigger = static_cast<u8>(packedState >> LEFT_TRIGGER_SHIFT);
	const u8 rightTrigger = static_cast<u8>(packedState >> RIGHT_TRIGGER_SHIFT);

	std::wstring result = getPrefix(style);
	bool hasInput = false;
	for (const auto &[mask, input] : XINPUT_BUTTONS) {
		if ((buttons & mask) != 0) {
			appendButton(result, getLabel(input, style));
			hasInput = true;
		}
	}

	if (leftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		appendButton(result, getLabel(LogicalInput::LeftTrigger, style));
		hasInput = true;
	}
	if (rightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		appendButton(result, getLabel(LogicalInput::RightTrigger, style));
		hasInput = true;
	}

	if ((buttons & XINPUT_GAMEPAD_DPAD_UP) != 0) {
		appendButton(result, L"D-Up");
		hasInput = true;
	}
	if ((buttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0) {
		appendButton(result, L"D-Right");
		hasInput = true;
	}
	if ((buttons & XINPUT_GAMEPAD_DPAD_DOWN) != 0) {
		appendButton(result, L"D-Down");
		hasInput = true;
	}
	if ((buttons & XINPUT_GAMEPAD_DPAD_LEFT) != 0) {
		appendButton(result, L"D-Left");
		hasInput = true;
	}

	if (!hasInput) {
		result += L" -";
	}
	return result;
}

bool appendPov(std::wstring &result, u32 pov) {
	if (pov == ~0u) {
		return false;
	}

	pov %= 36000;
	if (pov >= 31500 || pov <= 4500) {
		appendButton(result, L"D-Up");
	}
	if (pov >= 4500 && pov <= 13500) {
		appendButton(result, L"D-Right");
	}
	if (pov >= 13500 && pov <= 22500) {
		appendButton(result, L"D-Down");
	}
	if (pov >= 22500 && pov <= 31500) {
		appendButton(result, L"D-Left");
	}
	return true;
}

bool isBindingActive(const ControllerInput::DirectInputSnapshot &state, const InputBinding &binding) {
	if (binding.type == BindingType::Button) {
		return binding.index < state.buttons.size() && (state.buttons[binding.index] & 0x80) != 0;
	}
	if (binding.index >= state.axes.size()) {
		return false;
	}

	const int delta = static_cast<int>(state.axes[binding.index]) - static_cast<int>(binding.baseline);
	if (binding.type == BindingType::AxisPositive) {
		return delta > AXIS_INPUT_THRESHOLD;
	}
	return delta < -static_cast<int>(AXIS_INPUT_THRESHOLD);
}

std::wstring formatDirectInputButtons(
		const ControllerInput::DirectInputSnapshot &state, ControllerInput::DisplayStyle style,
		const std::array<InputBinding, LOGICAL_INPUT_COUNT> &bindings
) {
	std::wstring result = getPrefix(style);
	bool hasInput = false;
	for (size_t index = 0; index < bindings.size(); ++index) {
		if (isBindingActive(state, bindings[index])) {
			appendButton(result, getLabel(static_cast<LogicalInput>(index), style));
			hasInput = true;
		}
	}

	hasInput = appendPov(result, state.pov) || hasInput;
	if (!hasInput) {
		result += L" -";
	}
	return result;
}

bool anyButtonPressed(const ControllerInput::DirectInputSnapshot &state) {
	return std::any_of(state.buttons.begin(), state.buttons.end(), [](u8 value) { return (value & 0x80) != 0; });
}

bool controlsReleased(const ControllerInput::DirectInputSnapshot &state, const CalibrationState &stateMachine) {
	if (anyButtonPressed(state)) {
		return false;
	}
	for (size_t index = 0; index < state.axes.size(); ++index) {
		const int delta = static_cast<int>(state.axes[index]) - static_cast<int>(stateMachine.baseline[index]);
		if (delta > AXIS_INPUT_THRESHOLD || delta < -static_cast<int>(AXIS_INPUT_THRESHOLD)) {
			return false;
		}
	}
	return true;
}

std::optional<InputBinding> findPressedButton(const ControllerInput::DirectInputSnapshot &state) {
	for (size_t index = 0; index < state.buttons.size(); ++index) {
		if ((state.buttons[index] & 0x80) != 0) {
			return InputBinding{BindingType::Button, static_cast<u8>(index), 0};
		}
	}
	return std::nullopt;
}

std::optional<InputBinding> findMovedAxis(
		const ControllerInput::DirectInputSnapshot &state, const CalibrationState &stateMachine
) {
	int largestMagnitude = AXIS_INPUT_THRESHOLD;
	int selectedDelta = 0;
	size_t selectedIndex = state.axes.size();
	for (size_t index = 0; index < state.axes.size(); ++index) {
		const int delta = static_cast<int>(state.axes[index]) - static_cast<int>(stateMachine.baseline[index]);
		const int magnitude = delta < 0 ? -delta : delta;
		if (magnitude > largestMagnitude) {
			largestMagnitude = magnitude;
			selectedDelta = delta;
			selectedIndex = index;
		}
	}

	if (selectedIndex == state.axes.size()) {
		return std::nullopt;
	}
	return InputBinding{
			selectedDelta > 0 ? BindingType::AxisPositive : BindingType::AxisNegative,
			static_cast<u8>(selectedIndex), stateMachine.baseline[selectedIndex]
	};
}

u32 encodeBinding(const InputBinding &binding) {
	return static_cast<u32>(binding.type) << 28 | static_cast<u32>(binding.baseline) << 8 | binding.index;
}

std::optional<InputBinding> decodeBinding(u32 encoded) {
	const auto type = static_cast<BindingType>((encoded >> 28) & 0x0F);
	const u8 index = static_cast<u8>(encoded & 0xFF);
	const u16 baseline = static_cast<u16>((encoded >> 8) & 0xFFFF);
	if (type == BindingType::Button && index < 128) {
		return InputBinding{type, index, 0};
	}
	if ((type == BindingType::AxisPositive || type == BindingType::AxisNegative) && index < 6) {
		return InputBinding{type, index, baseline};
	}
	return std::nullopt;
}

std::array<u32, LOGICAL_INPUT_COUNT> encodeBindings(
		const std::array<InputBinding, LOGICAL_INPUT_COUNT> &bindings
) {
	std::array<u32, LOGICAL_INPUT_COUNT> encoded{};
	for (size_t index = 0; index < bindings.size(); ++index) {
		encoded[index] = encodeBinding(bindings[index]);
	}
	return encoded;
}

void loadPersistedBindings() {
#ifndef CONTROLLER_INPUT_TESTING
	std::array<u32, LOGICAL_INPUT_COUNT> encoded{};
	if (!Persistence::getControllerMapping(encoded.data(), encoded.size())) {
		return;
	}

	std::array<InputBinding, LOGICAL_INPUT_COUNT> loaded{};
	for (size_t index = 0; index < encoded.size(); ++index) {
		const auto binding = decodeBinding(encoded[index]);
		if (!binding.has_value()) {
			return;
		}
		loaded[index] = *binding;
	}
	std::lock_guard<std::mutex> lock(directInputMutex);
	inputBindings = loaded;
#endif
}

void saveBindings(const std::array<InputBinding, LOGICAL_INPUT_COUNT> &bindings) {
#ifndef CONTROLLER_INPUT_TESTING
	const auto encoded = encodeBindings(bindings);
	Persistence::setControllerMapping(encoded.data(), encoded.size());
#endif
}

std::optional<std::array<InputBinding, LOGICAL_INPUT_COUNT>> updateCalibration(
		const ControllerInput::DirectInputSnapshot &state
) {
	if (!calibration.active) {
		return std::nullopt;
	}

	if (!calibration.baselineCaptured) {
		if (anyButtonPressed(state)) {
			return std::nullopt;
		}
		calibration.baseline = state.axes;
		calibration.baselineCaptured = true;
		calibration.waitingForRelease = false;
		return std::nullopt;
	}

	if (calibration.waitingForRelease) {
		if (controlsReleased(state, calibration)) {
			calibration.waitingForRelease = false;
		}
		return std::nullopt;
	}

	std::optional<InputBinding> binding = findPressedButton(state);
	const auto input = static_cast<LogicalInput>(calibration.step);
	if (!binding.has_value() && (input == LogicalInput::LeftTrigger || input == LogicalInput::RightTrigger)) {
		binding = findMovedAxis(state, calibration);
	}
	if (!binding.has_value()) {
		return std::nullopt;
	}

	calibration.workingBindings[calibration.step] = *binding;
	++calibration.step;
	calibration.waitingForRelease = true;
	if (calibration.step < LOGICAL_INPUT_COUNT) {
		return std::nullopt;
	}

	inputBindings = calibration.workingBindings;
	calibration.active = false;
	return inputBindings;
}

std::wstring getCalibrationText(ControllerInput::DisplayStyle style) {
	if (!calibration.baselineCaptured || calibration.waitingForRelease) {
		return L"Controller mapping: release all controls";
	}
	const auto input = static_cast<LogicalInput>(calibration.step);
	return L"Map [" + std::to_wstring(calibration.step + 1) + L"/" + std::to_wstring(LOGICAL_INPUT_COUNT) +
			L"]: press " + getLabel(input, style);
}

bool hasDirectInput(const ControllerInput::DirectInputSnapshot &state) {
	return state.pov != ~0u || anyButtonPressed(state);
}

BOOL CALLBACK findProcessWindow(HWND window, LPARAM parameter) {
	DWORD processId = 0;
	GetWindowThreadProcessId(window, &processId);
	if (processId != GetCurrentProcessId() || !IsWindowVisible(window)) {
		return TRUE;
	}

	*reinterpret_cast<HWND *>(parameter) = window;
	return FALSE;
}

HWND getProcessWindow() {
	HWND window = nullptr;
	EnumWindows(findProcessWindow, reinterpret_cast<LPARAM>(&window));
	return window;
}

void setAxisRange(IDirectInputDevice8W *device, DWORD axisOffset) {
	DIPROPRANGE range{};
	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.diph.dwObj = axisOffset;
	range.diph.dwHow = DIPH_BYOFFSET;
	range.lMin = 0;
	range.lMax = 65535;
	device->SetProperty(DIPROP_RANGE, &range.diph);
}

BOOL CALLBACK enumerateController(const DIDEVICEINSTANCEW *instance, VOID *parameter) {
	auto *context = reinterpret_cast<DeviceEnumerationContext *>(parameter);
	ComPtr<IDirectInputDevice8W> device;
	HRESULT result = context->directInput->CreateDevice(instance->guidInstance, device.GetAddressOf(), nullptr);
	if (FAILED(result)) {
		return DIENUM_CONTINUE;
	}

	result = device->SetDataFormat(&c_dfDIJoystick2);
	if (FAILED(result)) {
		return DIENUM_CONTINUE;
	}

	result = device->SetCooperativeLevel(context->window, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result)) {
		return DIENUM_CONTINUE;
	}

	for (DWORD offset : {DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ}) {
		setAxisRange(device.Get(), offset);
	}
	device->Acquire();
	context->devices->push_back({device});
	return DIENUM_CONTINUE;
}

std::vector<DirectInputDevice> enumerateControllers(IDirectInput8W *directInput) {
	std::vector<DirectInputDevice> devices;
	HWND window = getProcessWindow();
	if (!window) {
		return devices;
	}

	DeviceEnumerationContext context{directInput, window, &devices};
	directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, enumerateController, &context, DIEDFL_ATTACHEDONLY);
	return devices;
}

u16 normalizeAxis(LONG value) { return static_cast<u16>(std::clamp<LONG>(value, 0, 65535)); }

bool pollController(DirectInputDevice &device, ControllerInput::DirectInputSnapshot &snapshot) {
	HRESULT result = device.device->Poll();
	if (FAILED(result)) {
		result = device.device->Acquire();
		while (result == DIERR_INPUTLOST) {
			result = device.device->Acquire();
		}
		if (FAILED(result)) {
			return false;
		}
	}

	DIJOYSTATE2 state{};
	result = device.device->GetDeviceState(sizeof(state), &state);
	if (FAILED(result)) {
		return false;
	}

	std::copy(std::begin(state.rgbButtons), std::end(state.rgbButtons), snapshot.buttons.begin());
	snapshot.axes = {
			normalizeAxis(state.lX), normalizeAxis(state.lY), normalizeAxis(state.lZ),
			normalizeAxis(state.lRx), normalizeAxis(state.lRy), normalizeAxis(state.lRz)
	};
	snapshot.pov = state.rgdwPOV[0];
	return true;
}

void runDirectInputPoller() {
	ComPtr<IDirectInput8W> directInput;
	HRESULT result = DirectInput8Create(
			GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8W,
			reinterpret_cast<void **>(directInput.GetAddressOf()), nullptr
	);
	if (FAILED(result)) {
		ControllerInput::clearDirectInput();
		return;
	}

	std::vector<DirectInputDevice> devices;
	auto nextDeviceScan = std::chrono::steady_clock::now();
	while (pollerRunning.load(std::memory_order_acquire)) {
		const auto now = std::chrono::steady_clock::now();
		if (now >= nextDeviceScan) {
			devices = enumerateControllers(directInput.Get());
			nextDeviceScan = now + DEVICE_RESCAN_INTERVAL;
		}

		bool receivedState = false;
		ControllerInput::DirectInputSnapshot selectedState;
		for (auto &device : devices) {
			ControllerInput::DirectInputSnapshot state;
			if (!pollController(device, state)) {
				continue;
			}

			if (!receivedState || hasDirectInput(state)) {
				selectedState = state;
				receivedState = true;
			}
			if (hasDirectInput(state)) {
				break;
			}
		}

		if (receivedState) {
			ControllerInput::updateDirectInput(selectedState);
		} else {
			ControllerInput::clearDirectInput();
		}
		std::this_thread::sleep_for(POLL_INTERVAL);
	}

	for (auto &device : devices) {
		device.device->Unacquire();
	}
	ControllerInput::clearDirectInput();
}

} // namespace

namespace AutomataMod::ControllerInput {

void start() {
	loadPersistedBindings();
	bool expected = false;
	if (!pollerRunning.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
		return;
	}
	pollerThread = std::thread(runDirectInputPoller);
}

void stop() {
	pollerRunning.store(false, std::memory_order_release);
	if (pollerThread.joinable()) {
		pollerThread.join();
	}
	clearDirectInput();
}

void setDisplayStyle(DisplayStyle style) { displayStyle.store(style, std::memory_order_release); }

DisplayStyle getDisplayStyle() { return displayStyle.load(std::memory_order_acquire); }

DisplayStyle toggleDisplayStyle() {
	const DisplayStyle next = getDisplayStyle() == DisplayStyle::Xbox ? DisplayStyle::PlayStation : DisplayStyle::Xbox;
	setDisplayStyle(next);
	return next;
}

void startCalibration() {
	std::lock_guard<std::mutex> lock(directInputMutex);
	calibration = {};
	calibration.active = true;
}

bool isCalibrationActive() {
	std::lock_guard<std::mutex> lock(directInputMutex);
	return calibration.active;
}

void updateXInput(u16 buttons, u8 leftTrigger, u8 rightTrigger) {
	u32 packedState = buttons;
	packedState |= static_cast<u32>(leftTrigger) << LEFT_TRIGGER_SHIFT;
	packedState |= static_cast<u32>(rightTrigger) << RIGHT_TRIGGER_SHIFT;
	xinputState.store(packedState, std::memory_order_relaxed);
	xinputConnected.store(true, std::memory_order_release);
}

void clearXInput() {
	xinputConnected.store(false, std::memory_order_release);
	xinputState.store(0, std::memory_order_relaxed);
}

void updateDirectInput(const DirectInputSnapshot &state) {
	std::optional<std::array<InputBinding, LOGICAL_INPUT_COUNT>> completedMapping;
	{
		std::lock_guard<std::mutex> lock(directInputMutex);
		directInputState = state;
		directInputConnected = true;
		completedMapping = updateCalibration(state);
	}
	if (completedMapping.has_value()) {
		saveBindings(*completedMapping);
	}
}

void clearDirectInput() {
	std::lock_guard<std::mutex> lock(directInputMutex);
	directInputState = {};
	directInputConnected = false;
}

std::wstring getPressedButtons() {
	const DisplayStyle style = getDisplayStyle();
	{
		std::lock_guard<std::mutex> lock(directInputMutex);
		if (calibration.active) {
			return getCalibrationText(style);
		}
	}

	if (xinputConnected.load(std::memory_order_acquire)) {
		return formatXInputButtons(xinputState.load(std::memory_order_relaxed), style);
	}

	std::lock_guard<std::mutex> lock(directInputMutex);
	return directInputConnected ? formatDirectInputButtons(directInputState, style, inputBindings)
									 : getPrefix(style) + L" -";
}

} // namespace AutomataMod::ControllerInput
