#include <Windows.h>
#include <Xinput.h>
#include <bcrypt.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <winuser.h>

#include "AutomataMod.hpp"
#include "com/FactoryWrapper2.hpp"
#include "com/WrapperPointer.hpp"
#include "infra/DLL.hpp"
#include "infra/HashCheck.hpp"
#include "infra/IAT.hpp"
#include "infra/Log.hpp"
#include "infra/ModConfig.hpp"
#include "infra/constants.hpp"

using namespace AutomataMod;

namespace {

WORD lastXInputButtons = 0;
bool kbInputSetModActiveFlag = true;
std::unique_ptr<DLL> xinput;
std::unique_ptr<std::thread> checkerThread;
std::unique_ptr<std::thread> keyboardInputThread;
std::unique_ptr<IAT::IATHook> d3dCreateDeviceHook;
std::unique_ptr<IAT::IATHook> dxgiCreateFactoryHook;
WrapperPointer<DxWrappers::DXGIFactoryWrapper2> factory;
bool shouldStopChecker = false;

// Need to intercept the DXGI factory to return our wrapper of it
HRESULT WINAPI CreateDXGIFactoryHooked(REFIID riid, void **ppFactory) {
	log(LogLevel::LOG_INFO, "CreateDXGIFactory called");

	if (riid != __uuidof(IDXGIFactory2)) {
		log(LogLevel::LOG_ERROR, "Unknown IDXGIFactory being used by automata. This will probably crash.");
	}

	Microsoft::WRL::ComPtr<IDXGIFactory2> pf;
	HRESULT facResult = CreateDXGIFactory(riid, reinterpret_cast<void **>(pf.GetAddressOf()));
	if (SUCCEEDED(facResult)) {
		factory = new DxWrappers::DXGIFactoryWrapper2(pf);
		(*(IDXGIFactory2 **)ppFactory) = factory.get();
		log(LogLevel::LOG_INFO, "Created DXGIFactoryWrapper");
	} else {
		log(LogLevel::LOG_INFO, "Failed to create DXGIFactoryWrapper.");
	}

	return facResult;
}

// need to intercept the D3D11 create device call to add D2D support
HRESULT WINAPI D3D11CreateDeviceHooked(
		IDXGIAdapter *pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
		const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device **ppDevice,
		D3D_FEATURE_LEVEL *pFeatureLevel, ID3D11DeviceContext **ppImmediateContext
) {
	Flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	return D3D11CreateDevice(
			pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel,
			ppImmediateContext
	);
}

void handleKeyboardInput() {
	keyboardInputThread = std::unique_ptr<std::thread>(new std::thread([]() {
		while (!shouldStopChecker) {
			if (GetAsyncKeyState(VK_HOME) != 0 && kbInputSetModActiveFlag) {
				ModChecker *modChecker = ModChecker::get();
				modChecker->setModActive(!modChecker->getModActive());
				AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Mod toggled from keyboard input");
				kbInputSetModActiveFlag = false;
			} else if (GetAsyncKeyState(VK_HOME) == 0) {
				kbInputSetModActiveFlag = true;
			}

			std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(33));
		}
	}));
}

void init() {
	using namespace AutomataMod;

	log(LogLevel::LOG_INFO, "Initializing AutomataMod {}", AutomataMod::Constants::getVersion());
	u64 processRamStartAddr = reinterpret_cast<u64>(GetModuleHandle(nullptr));
	log(LogLevel::LOG_INFO, "Process ram start: 0x{:X}", processRamStartAddr);

	std::optional<NierVerisonInfo> version;
	try {
		version = QueryNierBinaryVersion();
	} catch (const std::runtime_error &e) {
		log(LogLevel::LOG_ERROR, "Error while calculating Nier binary hash: {}", e.what());
		return;
	}

	if (!version.has_value()) {
#ifdef _DEBUG
		log(LogLevel::LOG_ERROR, "Failed to determine Nier verison from executable. Defaulting to 1.02");
		version = NierVerisonInfo(NierVersion::NIERVER_102, "NieR:Automata (v1.02 defaulted)");
#else
		log(LogLevel::LOG_ERROR,
				"Unknown nier version detected. The mod doesn't know how to work with this version. Aborting.");
		return;
#endif
	}

	log(LogLevel::LOG_INFO, "Detected Nier version: {}", version.value().versionName());

	Addresses addresses;
	addresses.ramStart = processRamStartAddr;
	if (version == NierVersion::NIERVER_102) {
		addresses.currentPhase = 0xF64B10;
		addresses.isWorldLoaded = 0xF6E240;
		addresses.playerSetName = 0x124DE4C;
		addresses.isLoading = 0x14005F4;
		addresses.itemTableStart = 0x148C4C4;
		addresses.chipTableStart = 0x148E410;
		addresses.playerLocation = 0x12553E0;
		addresses.unitData = 0x14944C8;
		addresses.windowMode = 0x1421F38;
		addresses.stickState = 0x13fcc10;
	} else {
		log(LogLevel::LOG_ERROR, "Unsupported Nier version: {}", version.value().versionName());
		return;
	}

	AutomataMod::ModChecker::set(std::make_unique<ModChecker>(addresses));

	checkerThread = std::unique_ptr<std::thread>(new std::thread([]() {
		ModChecker *modChecker = ModChecker::get();
		while (!shouldStopChecker) {
			if (modChecker && factory)
				modChecker->checkStuff(factory.getComPtr());

			std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(250)); // check stuff 4 times a second
		}
	}));

	handleKeyboardInput();
}

template <typename FuncPtr> FuncPtr getXinputFunc(const std::string &funcName) {
	if (!xinput) {
		xinput = std::make_unique<DLL>(XINPUT_DLL_A);
		if (!xinput->isModuleFound()) {
			log(LogLevel::LOG_ERROR, "Failed to load {}. VC3 Mod and Automata will probably crash now.", XINPUT_DLL_A);
			return nullptr;
		}
	}

	return xinput->getFunc<FuncPtr>(funcName);
}

} // namespace

extern "C" {

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) {
		d3dCreateDeviceHook = std::unique_ptr<IAT::IATHook>(
				new IAT::IATHook("d3d11.dll", "D3D11CreateDevice", (LPCVOID)D3D11CreateDeviceHooked)
		);
		dxgiCreateFactoryHook = std::unique_ptr<IAT::IATHook>(
				new IAT::IATHook("dxgi.dll", "CreateDXGIFactory", (LPCVOID)CreateDXGIFactoryHooked)
		);

		shouldStopChecker = false;
		init();
	} else if (reason == DLL_PROCESS_DETACH) {
		shouldStopChecker = true;
		if (checkerThread) {
			checkerThread->join();
			checkerThread = nullptr;
		}

		if (keyboardInputThread) {
			keyboardInputThread->join();
			keyboardInputThread = nullptr;
		}

		AutomataMod::ModChecker::set(nullptr);
		factory = nullptr;
		d3dCreateDeviceHook = nullptr;
		dxgiCreateFactoryHook = nullptr;
		xinput = nullptr;
	}

	return TRUE;
}

void WINAPI XInputEnable(_In_ BOOL enable) WIN_NOEXCEPT {
	static auto ptr = getXinputFunc<void(WINAPI *)(BOOL)>("XInputEnable");
	if (!ptr)
		return;
	ptr(enable);
}

DWORD WINAPI XInputGetAudioDeviceIds(
		_In_ DWORD dwUserIndex, _Out_writes_opt_(*pRenderCount) LPWSTR pRenderDeviceId, _Inout_opt_ UINT *pRenderCount,
		_Out_writes_opt_(*pCaptureCount) LPWSTR pCaptureDeviceId, _Inout_opt_ UINT *pCaptureCount
) WIN_NOEXCEPT {
	static auto ptr = getXinputFunc<DWORD(WINAPI *)(DWORD, LPWSTR, UINT *, LPWSTR, UINT *)>("XInputGetAudioDeviceIds");
	if (!ptr)
		return ERROR_DEVICE_NOT_CONNECTED;
	return ptr(dwUserIndex, pRenderDeviceId, pRenderCount, pCaptureDeviceId, pCaptureCount);
}

DWORD WINAPI XInputGetBatteryInformation(
		_In_ DWORD dwUserIndex, _In_ BYTE devType, _Out_ XINPUT_BATTERY_INFORMATION *pBatteryInformation
) WIN_NOEXCEPT {
	static auto ptr =
			getXinputFunc<DWORD(WINAPI *)(DWORD, BYTE, XINPUT_BATTERY_INFORMATION *)>("XInputGetBatteryInformation");
	if (!ptr)
		return ERROR_DEVICE_NOT_CONNECTED;
	return ptr(dwUserIndex, devType, pBatteryInformation);
}

DWORD WINAPI XInputGetCapabilities(_In_ DWORD dwUserIndex, _In_ DWORD dwFlags, _Out_ XINPUT_CAPABILITIES *pCapabilities)
		WIN_NOEXCEPT {
	static auto ptr = getXinputFunc<DWORD(WINAPI *)(DWORD, DWORD, XINPUT_CAPABILITIES *)>("XInputGetCapabilities");
	if (!ptr)
		return ERROR_DEVICE_NOT_CONNECTED;
	return ptr(dwUserIndex, dwFlags, pCapabilities);
}

DWORD WINAPI XInputGetKeystroke(_In_ DWORD dwUserIndex, _Reserved_ DWORD dwReserved, _Out_ PXINPUT_KEYSTROKE pKeystroke)
		WIN_NOEXCEPT {
	static auto ptr = getXinputFunc<DWORD(WINAPI *)(DWORD, DWORD, XINPUT_KEYSTROKE *)>("XInputGetKeystroke");
	if (!ptr)
		return ERROR_DEVICE_NOT_CONNECTED;
	return ptr(dwUserIndex, dwReserved, pKeystroke);
}

DWORD WINAPI XInputGetState(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState) WIN_NOEXCEPT {
	static auto ptr = getXinputFunc<DWORD(WINAPI *)(DWORD, XINPUT_STATE *)>("XInputGetState");
	if (!ptr)
		return ERROR_DEVICE_NOT_CONNECTED;

	DWORD result = ptr(dwUserIndex, pState);

	// Check for mod toggle button combo
	ModChecker *modChecker = ModChecker::get();
	if (result == ERROR_SUCCESS && dwUserIndex == 0 && modChecker && pState && !modChecker->validCheckState()) {
		// check if new state diff from last state
		if (pState->Gamepad.wButtons != lastXInputButtons) {
			WORD x = pState->Gamepad.wButtons & XINPUT_GAMEPAD_X;
			WORD y = pState->Gamepad.wButtons & XINPUT_GAMEPAD_Y;
			if (x && y)
				modChecker->setModActive(!modChecker->getModActive());
			lastXInputButtons = pState->Gamepad.wButtons;
		}
	}

	return result;
}

DWORD WINAPI XInputSetState(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION *pVibration) WIN_NOEXCEPT {
	static auto ptr = getXinputFunc<DWORD(WINAPI *)(DWORD, XINPUT_VIBRATION *)>("XInputSetState");
	if (!ptr)
		return ERROR_DEVICE_NOT_CONNECTED;
	return ptr(dwUserIndex, pVibration);
}

} // extern "C"
