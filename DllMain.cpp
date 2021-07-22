#include <windows.h>
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

namespace {

DLLHook dinput;

std::unique_ptr<std::thread> checkerThread;
std::unique_ptr<IAT::IATHook> d3dCreateDeviceHook;
std::unique_ptr<IAT::IATHook> dxgiCreateFactoryHook;

CComPtr<DxWrappers::DXGIFactoryWrapper> wrapper;

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
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Initializing AutomataMod v1.7");
    uint64_t processRamStartAddr = reinterpret_cast<uint64_t>(GetModuleHandle(nullptr));
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Process ram start: " + std::to_string(processRamStartAddr));
    checkerThread = std::unique_ptr<std::thread>(new std::thread([processRamStartAddr]() {
        AutomataMod::checkStuff(processRamStartAddr);
    }));
}

template<typename FuncPtr>
FuncPtr hookFunc(const std::string& funcName) {
    if (!dinput) {
        dinput = DLLHook("dinput8.dll");
        if (!dinput) {
            return nullptr;
        }
    }

    return dinput.hookFunc<FuncPtr>(funcName);
}

} // namespace

extern "C" {
    BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
        if (reason == DLL_PROCESS_ATTACH) {
            d3dCreateDeviceHook = std::unique_ptr<IAT::IATHook>(new IAT::IATHook("d3d11.dll", "D3D11CreateDevice", (LPCVOID)D3D11CreateDeviceHooked));
            dxgiCreateFactoryHook = std::unique_ptr<IAT::IATHook>(new IAT::IATHook("dxgi.dll", "CreateDXGIFactory", (LPCVOID)CreateDXGIFactoryHooked));
            init();
        } else if (reason == DLL_PROCESS_DETACH) {
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

    HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter) {
        auto ptr = hookFunc<HRESULT(WINAPI*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN)>("DirectInput8Create");
        if (!ptr) return E_FAIL;
        return ptr(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    }
}
