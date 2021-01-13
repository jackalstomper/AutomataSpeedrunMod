#include <windows.h>
#include <thread>
#include <d3d11.h>
#include <stdexcept>
#include <vector>
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

void modPhaseJumpMethod(uint64_t processRamStartAddr) {
    // Modify all calls to phasejump to use the fastload flag.
    // Do this by inject a jump function to our custom code that sets the field to 0, then jumping back
    std::vector<uint8_t> fastLoadCode = {
        0x41, 0x83, 0x09, 0x02,     // or dword ptr [r9],02; Sets the load to use the no text screen
        0x53,                       // push rbx; an instruction we overwrote to make the new jmp
        0x56,                       // push rsi; an instruction we overwrote to make the new jmp
        0xEB, 0x0B                  // jmp NieRAutomata.exe+51A734; Go back to original code
    };

    std::vector<uint8_t> jumpCode = {
        0xEB, 0xED // jmp NieRAutomata.exe+51A721
    };

    // replace two instructions starting at this location with a jmp to our code
    // we'll repeat those instructions in our code
    uint64_t codeChangeTargetLocation = processRamStartAddr + 0x51A732;

    // A code cave between fuctions full of int 3(0xCC) bytes
    uint64_t newCodeLocation = processRamStartAddr + 0x51A721;

    void* newCodePointer = reinterpret_cast<void*>(newCodeLocation);

    DWORD oldProtection;
    if (VirtualProtect(newCodePointer, 19, PAGE_READWRITE, &oldProtection) == 0) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to fetch memory info for automata. We can't modify the phaseJump code.");
        throw new std::runtime_error("Failed to fetch memory info for automata. We can't modify the phaseJump code.");
    }

    // write new code in the cave
    memcpy(newCodePointer, fastLoadCode.data(), fastLoadCode.size());

    // inject new jmp instruction into phasejump method to point to our new code
    memcpy(reinterpret_cast<char*>(codeChangeTargetLocation), jumpCode.data(), jumpCode.size());

    DWORD dummy;
    if (VirtualProtect(newCodePointer, 19, oldProtection, &dummy) == 0) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to fetch memory info for automata. We can't modify the phaseJump code.");
        throw new std::runtime_error("Failed to fetch memory info for automata. We can't modify the phaseJump code.");
    }
}

void init() {
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Initializing AutomataMod v1.7");
    uint64_t processRamStartAddr = reinterpret_cast<uint64_t>(GetModuleHandle(nullptr));
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Process ram start: " + std::to_string(processRamStartAddr));

    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Modifying phasejump function for faster loads");
    modPhaseJumpMethod(processRamStartAddr);

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
