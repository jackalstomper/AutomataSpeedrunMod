#include <d3d11.h>
#include <atlbase.h>
#include <string>
#include <memory>
#include "iat.hpp"
#include "DLLHook.hpp"
#include "Log.hpp"
#include "FactoryWrapper.hpp"

namespace {

DLLHook dinput;
std::unique_ptr<IAT::IATHook> dxgiCreateFactoryHook;
CComPtr<DxWrappers::DXGIFactoryWrapper> factoryWrapper;

HRESULT WINAPI CreateDXGIFactoryHooked(REFIID riid, void** ppFactory) {
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "CreateDXGIFactory called");

    if (riid != __uuidof(IDXGIFactory2)) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Unknown IDXGIFactory being used by automata. This will probably crash.");
    }

    CComPtr<IDXGIFactory2> factory;
    HRESULT facResult = CreateDXGIFactory(riid, (void**)&factory);
    if (SUCCEEDED(facResult)) {
        factoryWrapper = new DxWrappers::DXGIFactoryWrapper(factory);
        (*(IDXGIFactory2**)ppFactory) = (IDXGIFactory2*)factoryWrapper;
        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Created DXGIFactoryWrapper");
    } else {
        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Failed to create DXGIFactoryWrapper.");
    }

    return facResult;
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
            dxgiCreateFactoryHook = std::unique_ptr<IAT::IATHook>(new IAT::IATHook("dxgi.dll", "CreateDXGIFactory", (LPCVOID)CreateDXGIFactoryHooked));
        } else if (reason == DLL_PROCESS_DETACH) {
            factoryWrapper = nullptr;
            dxgiCreateFactoryHook = nullptr;
        }

        return TRUE;
    }

    HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter) {
        auto ptr = hookFunc<HRESULT(WINAPI*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN)>("DirectInput8Create");
        if (!ptr) return E_FAIL;
        return ptr(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    }
}
