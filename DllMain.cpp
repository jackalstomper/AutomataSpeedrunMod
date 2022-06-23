#include <windows.h>
#include <Psapi.h>
#include <thread>
#include <vector>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <string>
#include "AutomataMod.hpp"
#include "Log.hpp"
#include "iat.hpp"
#include "FactoryWrapper.hpp"
#include "DLLHook.hpp"
#include "ModConfig.hpp"

namespace {

std::unique_ptr<DLLHook> xinput;
std::unique_ptr<std::thread> checkerThread;
std::unique_ptr<IAT::IATHook> d3dCreateDeviceHook;
std::unique_ptr<IAT::IATHook> dxgiCreateFactoryHook;
std::unique_ptr<AutomataMod::ModChecker> modChecker;
CComPtr<DxWrappers::DXGIFactoryWrapper> wrapper;
bool shouldStopChecker = false;

uint64_t getModuleSize() {
    MODULEINFO info;
    BOOL result = GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &info, sizeof(info));
    return info.SizeOfImage;
}

// Need to intercept the DXGI factory to return our wrapper of it
HRESULT WINAPI CreateDXGIFactoryHooked(REFIID riid, void** ppFactory) {
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "CreateDXGIFactory called");

    if (riid != __uuidof(IDXGIFactory2)) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Unknown IDXGIFactory being used by automata. This will probably crash.");
    }

    CComPtr<IDXGIFactory2> factory;
    HRESULT facResult = CreateDXGIFactory(riid, (void**)&factory);
    if (SUCCEEDED(facResult)) {
        wrapper = new DxWrappers::DXGIFactoryWrapper(factory);
        (*(IDXGIFactory2**)ppFactory) = (IDXGIFactory2*)wrapper;
        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Created DXGIFactoryWrapper");
    } else {
        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Failed to create DXGIFactoryWrapper.");
    }

    return facResult;
}

// need to intercept the D3D11 create device call to add D2D support
HRESULT WINAPI D3D11CreateDeviceHooked(
    IDXGIAdapter* pAdapter,
    D3D_DRIVER_TYPE         DriverType,
    HMODULE                 Software,
    UINT                    Flags,
    const D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT                    FeatureLevels,
    UINT                    SDKVersion,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel,
    ID3D11DeviceContext** ppImmediateContext
) {
    return D3D11CreateDevice(
        pAdapter,
        DriverType,
        Software,
        Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        pFeatureLevels,
        FeatureLevels,
        SDKVersion,
        ppDevice,
        pFeatureLevel,
        ppImmediateContext);
}

void init() {
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Initializing AutomataMod v1.9");
    uint64_t processRamStartAddr = reinterpret_cast<uint64_t>(GetModuleHandle(nullptr));
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Process ram start: " + std::to_string(processRamStartAddr));

    AutomataMod::ModConfig modConfig;
    uint64_t moduleSize = getModuleSize();
    if (moduleSize == 26177536) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Detected Automata version 1.02");
        AutomataMod::ModConfig::Addresses addresses;
        addresses.currentPhase = 0xF64B10;
        addresses.isWorldLoaded = 0xF6E240;
        addresses.playerSetName = 0x124DE4C;
        addresses.isLoading = 0x100D410;
        addresses.itemTableStart = 0x148C4C4;
        addresses.chipTableStart = 0x148E410;
        addresses.playerLocation = 0x12553E0;
        addresses.unitData = 0x14944C8;
        modConfig.setAddresses(std::move(addresses));
    } else {
        AutomataMod::logEx(AutomataMod::LogLevel::LOG_ERROR, "Could not determine what automata version we are. VC3Mod will not boot. moduleSize: {}", moduleSize);
        return;
    }

    modChecker = std::unique_ptr<AutomataMod::ModChecker>(new AutomataMod::ModChecker(processRamStartAddr, std::move(modConfig)));

    checkerThread = std::unique_ptr<std::thread>(new std::thread([processRamStartAddr]() {
        while (!shouldStopChecker) {
            modChecker->checkStuff(wrapper);
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(250)); // check stuff 4 times a second
        }
    }));
}

template<typename FuncPtr>
FuncPtr hookFunc(const std::string& funcName) {
    if (!xinput) {
        xinput = std::unique_ptr<DLLHook>(new DLLHook("xinput1_4.dll"));
        if (!xinput->isModuleFound()) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to load xinput1_4.dll. VC3 Mod and Automata will probably crash now.");
            return nullptr;
        }
    }

    return xinput->hookFunc<FuncPtr>(funcName);
}

} // namespace

extern "C" {
    BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
        if (reason == DLL_PROCESS_ATTACH) {
            d3dCreateDeviceHook = std::unique_ptr<IAT::IATHook>(new IAT::IATHook("d3d11.dll", "D3D11CreateDevice", (LPCVOID)D3D11CreateDeviceHooked));
            dxgiCreateFactoryHook = std::unique_ptr<IAT::IATHook>(new IAT::IATHook("dxgi.dll", "CreateDXGIFactory", (LPCVOID)CreateDXGIFactoryHooked));
            shouldStopChecker = false;
            init();
        } else if (reason == DLL_PROCESS_DETACH) {
            shouldStopChecker = true;

            if (d3dCreateDeviceHook) {
                d3dCreateDeviceHook = nullptr;
            }

            if (dxgiCreateFactoryHook) {
                dxgiCreateFactoryHook = nullptr;
            }

            if (checkerThread) {
                checkerThread = nullptr;
            }

            if (wrapper) {
                wrapper = nullptr;
            }
        }

        return TRUE;
    }

    void WINAPI XInputEnable(BOOL enable) {
        static auto ptr = hookFunc<void(WINAPI*)(BOOL)>("XInputEnable");
        if (!ptr) return;
        ptr(enable);
    }

    DWORD WINAPI XInputGetAudioDeviceIds(
        DWORD  dwUserIndex,
        LPWSTR pRenderDeviceId,
        UINT* pRenderCount,
        LPWSTR pCaptureDeviceId,
        UINT* pCaptureCount
    ) {
        static auto ptr = hookFunc<DWORD(WINAPI*)(DWORD, LPWSTR, UINT*, LPWSTR, UINT*)>("XInputGetAudioDeviceIds");
        if (!ptr) return ERROR_DEVICE_NOT_CONNECTED;
        return ptr(dwUserIndex, pRenderDeviceId, pRenderCount, pCaptureDeviceId, pCaptureCount);
    }

    struct XINPUT_BATTERY_INFORMATION;
    DWORD WINAPI XInputGetBatteryInformation(
        DWORD                      dwUserIndex,
        BYTE                       devType,
        XINPUT_BATTERY_INFORMATION* pBatteryInformation
    ) {
        static auto ptr = hookFunc<DWORD(WINAPI*)(DWORD, BYTE, XINPUT_BATTERY_INFORMATION*)>("XInputGetBatteryInformation");
        if (!ptr) return ERROR_DEVICE_NOT_CONNECTED;
        return ptr(dwUserIndex, devType, pBatteryInformation);
    }

    struct XINPUT_CAPABILITIES;
    DWORD WINAPI XInputGetCapabilities(
        DWORD               dwUserIndex,
        DWORD               dwFlags,
        XINPUT_CAPABILITIES* pCapabilities
    ) {
        static auto ptr = hookFunc<DWORD(WINAPI*)(DWORD, DWORD, XINPUT_CAPABILITIES*)>("XInputGetCapabilities");
        if (!ptr) return ERROR_DEVICE_NOT_CONNECTED;
        return ptr(dwUserIndex, dwFlags, pCapabilities);
    }

    struct XINPUT_KEYSTROKE;
    DWORD WINAPI XInputGetKeystroke(
        DWORD             dwUserIndex,
        DWORD             dwReserved,
        XINPUT_KEYSTROKE* pKeystroke
    ) {
        static auto ptr = hookFunc<DWORD(WINAPI*)(DWORD, DWORD, XINPUT_KEYSTROKE*)>("XInputGetKeystroke");
        if (!ptr) return ERROR_DEVICE_NOT_CONNECTED;
        return ptr(dwUserIndex, dwReserved, pKeystroke);
    }

    struct XINPUT_STATE;
    DWORD WINAPI XInputGetState(
        DWORD        dwUserIndex,
        XINPUT_STATE* pState
    ) {
        static auto ptr = hookFunc<DWORD(WINAPI*)(DWORD, XINPUT_STATE*)>("XInputGetState");
        if (!ptr) return ERROR_DEVICE_NOT_CONNECTED;
        return ptr(dwUserIndex, pState);
    }

    struct XINPUT_VIBRATION;
    DWORD WINAPI XInputSetState(
        DWORD            dwUserIndex,
        XINPUT_VIBRATION* pVibration
    ) {
        static auto ptr = hookFunc<DWORD(WINAPI*)(DWORD, XINPUT_VIBRATION*)>("XInputSetState");
        if (!ptr) return ERROR_DEVICE_NOT_CONNECTED;
        return ptr(dwUserIndex, pVibration);
    }
}
