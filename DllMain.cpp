#include <windows.h>
#include <thread>
#include <d3d11.h>
#include "AutomataMod.hpp"
#include "Log.hpp"
#include "iat.hpp"
#include "DxWrappers.hpp"

DxWrappers::DXGIFactoryWrapper* g_wrapper = nullptr;

namespace {

std::thread* checkerThread = nullptr;
IAT::IATHook* d3dCreateDeviceHook = nullptr;
IAT::IATHook* dxgiCreateFactoryHook = nullptr;

// Need to intercept the DXGI factory to return our wrapper of it
HRESULT WINAPI CreateDXGIFactoryHooked(REFIID riid, void** ppFactory) {
    IDXGIFactory* factory;
    HRESULT facResult = CreateDXGIFactory(riid, (void**)&factory);
    if (SUCCEEDED(facResult)) {
        if (g_wrapper)
            delete g_wrapper;

        g_wrapper = new DxWrappers::DXGIFactoryWrapper(factory);
        (*(IDXGIFactory**)ppFactory) = g_wrapper;
    }

    return facResult;
}

// need to intercept the D3D11 create device call to add D2D support
HRESULT WINAPI D3D11CreateDeviceHooked(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType,
        HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice,
        D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) {
    return D3D11CreateDevice(pAdapter, DriverType, Software, Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}


void init() {
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Initializing AutomataMod v1.4");
    uint64_t processRamStartAddr = reinterpret_cast<uint64_t>(GetModuleHandle(nullptr));
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Process ram start: " + std::to_string(processRamStartAddr));
    checkerThread = new std::thread([processRamStartAddr]() {
        AutomataMod::checkStuff(processRamStartAddr);
    });
}

} // namespace

extern "C" {
    BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
    {
        if (reason == DLL_PROCESS_ATTACH) {
            d3dCreateDeviceHook = new IAT::IATHook("d3d11.dll", "D3D11CreateDevice", (LPCVOID)D3D11CreateDeviceHooked);
            dxgiCreateFactoryHook = new IAT::IATHook("dxgi.dll", "CreateDXGIFactory", (LPCVOID)CreateDXGIFactoryHooked);
            init();
        } else if (reason == DLL_PROCESS_DETACH) {
            if (d3dCreateDeviceHook) {
                delete d3dCreateDeviceHook;
                d3dCreateDeviceHook = nullptr;
            }

            if (dxgiCreateFactoryHook) {
                delete dxgiCreateFactoryHook;
                dxgiCreateFactoryHook = nullptr;
            }

            if (checkerThread) {
                delete checkerThread;
                checkerThread = nullptr;
            }

            if (g_wrapper) {
                delete g_wrapper;
                g_wrapper = nullptr;
            }
        }

        return TRUE;
    }
}
