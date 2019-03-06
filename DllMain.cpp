// Proxy DLL for DirectInput 8 to provide an entry point for our code to modify automata with
#include <windows.h>
#include <psapi.h>
#include <thread>
#include "AutomataMod.hpp"

namespace {

// Function pointer types for the DLL functions we have to proxy
using DirectInput8CreateProc = HRESULT(WINAPI *)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
using DllGetClassObjectProc = HRESULT(WINAPI *)(REFCLSID, REFIID, LPVOID *);
using DllCanUnloadNowProc = HRESULT(WINAPI *)();
using DllRegisterServerProc = HRESULT(WINAPI *)();
using DllUnregisterServerProc = HRESULT(WINAPI *)();

DllGetClassObjectProc dllGetClassObjectFunc = nullptr;
DllCanUnloadNowProc dllCanUnloadNowFunc = nullptr;
DirectInput8CreateProc directInput8CreateFunc = nullptr;
DllRegisterServerProc dllRegisterServerFunc = nullptr;
DllUnregisterServerProc dllUnregisterServerFunc = nullptr;

HMODULE dInput8 = nullptr;
HMODULE processRamStart = nullptr;
std::thread* checkerThread = nullptr;

template<typename T>
T getProc(const char* procName)
{
    return reinterpret_cast<T>(GetProcAddress(dInput8, procName));
}

void init()
{
    if (dInput8)
        return;

    // Get the starting memory address for automata
    HANDLE currentProcess = GetCurrentProcess();
    HMODULE hMod;
    DWORD cbNeeded;

    if (!EnumProcessModulesEx(currentProcess, &hMod, sizeof(hMod), &cbNeeded, LIST_MODULES_64BIT))
        return;

    processRamStart = hMod;

    // Before attempting to load the real DirectInput 8 see if user has FAR installed and use FAR's proxy instead
    dInput8 = LoadLibrary("dinput8_far");
    if (!dInput8) {
        // User doesn't have FAR, fetch the official windows DirectInput 8 library
        // Get windows directory for this system so we can load the real library
        TCHAR windir[1024];
        UINT dirLen = GetSystemDirectory(windir, 1024);
        if (dirLen == 0)
            return;

        // Append the dll directory to windows dir
        if (dirLen + 13 > 1024)
            return; // welp. filepath is too long for our memory buffer. Something dynamic should be done about this...later

        memcpy(windir + dirLen, "\\dinput8.dll\0", 13);
        dInput8 = LoadLibraryEx(windir, NULL, 0);
        if (!dInput8)
            return;
    }

    // Get addresses to the real functions so we can forward our calls to them
    directInput8CreateFunc = getProc<DirectInput8CreateProc>("DirectInput8Create");
    dllGetClassObjectFunc = getProc<DllGetClassObjectProc>("DllGetClassObject");
    dllCanUnloadNowFunc = getProc<DllCanUnloadNowProc>("DllCanUnloadNow");
    dllRegisterServerFunc = getProc<DllRegisterServerProc>("DllRegisterServer");
    dllUnregisterServerFunc = getProc<DllUnregisterServerProc>("DllUnregisterServer");
    checkerThread = new std::thread(AutomataMod::checkStuff, reinterpret_cast<uint8_t*>(processRamStart));
}

} // namespace

extern "C" {
    BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
    {
        if (reason == DLL_PROCESS_DETACH) {
            if (dInput8) {
                FreeLibrary(dInput8);
                dInput8 = nullptr;
            }

            if (checkerThread) {
                delete checkerThread;
                checkerThread = nullptr;
            }
        }

        return TRUE;
    }

    HRESULT WINAPI DirectInput8Create(
        HINSTANCE hinst,
        DWORD dwVersion,
        REFIID riidltf,
        LPVOID *ppvOut,
        LPUNKNOWN punkOuter)
    {
        init();
        if (!directInput8CreateFunc)
            return E_NOINTERFACE;

        return directInput8CreateFunc(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    }

    HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
    {
        init();
        if (!dllGetClassObjectFunc)
            return E_FAIL;

        return dllGetClassObjectFunc(rclsid, riid, ppv);
    }

    HRESULT WINAPI DllCanUnloadNow()
    {
        init();
        if (!dllCanUnloadNowFunc)
            return E_FAIL;

        return dllCanUnloadNowFunc();
    }

    HRESULT WINAPI DllRegisterServer()
    {
        init();
        if (!dllRegisterServerFunc)
            return E_FAIL;

        return dllRegisterServerFunc();
    }

    HRESULT WINAPI DllUnregisterServer()
    {
        init();
        if (!dllUnregisterServerFunc)
            return E_FAIL;

        return dllUnregisterServerFunc();
    }
}
