#include <windows.h>
#include <psapi.h>
#include <thread>
#include "AutomataMod.hpp"

namespace {

struct XINPUT_BATTERY_INFORMATION;
struct XINPUT_CAPABILITIES;
struct XINPUT_STATE;
struct XINPUT_VIBRATION;
struct XINPUT_KEYSTROKE;

// Function pointer types for the DLL functions we have to proxy
using XInputEnableProc = void(WINAPI *)(BOOL);
using XInputGetBatteryInformationProc = DWORD(WINAPI *)(DWORD, BYTE, XINPUT_BATTERY_INFORMATION*);
using XInputGetCapabilitiesProc = DWORD(WINAPI *)(DWORD, DWORD, XINPUT_CAPABILITIES*);
using XInputGetKeystrokeProc = DWORD(WINAPI *)(DWORD, DWORD, XINPUT_KEYSTROKE*);
using XInputGetStateProc = DWORD(WINAPI *)(DWORD, XINPUT_STATE*);
using XInputSetStateProc = DWORD(WINAPI *)(DWORD, XINPUT_VIBRATION*);

XInputEnableProc _XInputEnableProc = nullptr;
XInputGetBatteryInformationProc _XInputGetBatteryInformationProc = nullptr;
XInputGetCapabilitiesProc _XInputGetCapabilitiesProc = nullptr;
XInputGetKeystrokeProc _XInputGetKeystrokeProc = nullptr;
XInputGetStateProc _XInputGetStateProc = nullptr;
XInputSetStateProc _XInputSetStateProc = nullptr;

HMODULE hProxyHandle = nullptr;
HMODULE processRamStart = nullptr;
std::thread* checkerThread = nullptr;

template<typename T>
T getProc(const char* procName)
{
    return reinterpret_cast<T>(GetProcAddress(hProxyHandle, procName));
}

void init()
{
    if (hProxyHandle)
        return;

    // Get the starting memory address for automata
    HANDLE currentProcess = GetCurrentProcess();
    HMODULE hMod;
    DWORD cbNeeded;

    if (!EnumProcessModulesEx(currentProcess, &hMod, sizeof(hMod), &cbNeeded, LIST_MODULES_64BIT))
        return;

    processRamStart = hMod;

    // Get windows directory for this system so we can load the real library
    TCHAR windir[1024];
    UINT dirLen = GetSystemDirectory(windir, 1024);
    if (dirLen == 0)
        return;

    // Append the dll directory to windows dir
    if (dirLen + 15 > 1024)
        return; // welp. filepath is too long for our memory buffer. Something dynamic should be done about this...later

    memcpy(windir + dirLen, "\\xinput1_3.dll\0", 15);
    hProxyHandle = LoadLibraryEx(windir, NULL, 0);
    if (!hProxyHandle)
        return;

    // Get addresses to the real functions so we can forward our calls to them
    _XInputEnableProc = getProc<XInputEnableProc>("XInputEnable");
    _XInputGetBatteryInformationProc = getProc<XInputGetBatteryInformationProc>("XInputGetBatteryInformation");
    _XInputGetCapabilitiesProc = getProc<XInputGetCapabilitiesProc>("XInputGetCapabilities");
    _XInputGetKeystrokeProc = getProc<XInputGetKeystrokeProc>("XInputGetKeystroke");
    _XInputGetStateProc = getProc<XInputGetStateProc>("XInputGetState");
    _XInputSetStateProc = getProc<XInputSetStateProc>("XInputSetState");

    checkerThread = new std::thread(AutomataMod::checkStuff, reinterpret_cast<uint8_t*>(processRamStart));
}

} // namespace

extern "C" {
    BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
    {
        if (reason == DLL_PROCESS_DETACH) {
            if (hProxyHandle) {
                FreeLibrary(hProxyHandle);
                hProxyHandle = nullptr;
            }

            if (checkerThread) {
                delete checkerThread;
                checkerThread = nullptr;
            }
        }

        return TRUE;
    }

    void WINAPI XInputEnable(BOOL b)
    {
        init();
        if (!_XInputEnableProc)
            return;

        _XInputEnableProc(b);
    }

    DWORD WINAPI XInputGetBatteryInformation(DWORD d, BYTE b, XINPUT_BATTERY_INFORMATION* x)
    {
        init();
        if (!_XInputGetBatteryInformationProc)
            return ERROR;

        return _XInputGetBatteryInformationProc(d, b, x);
    }

    DWORD WINAPI XInputGetCapabilities(DWORD d1, DWORD d2, XINPUT_CAPABILITIES* x)
    {
        init();
        if (!_XInputGetCapabilitiesProc)
            return ERROR;

        return _XInputGetCapabilitiesProc(d1, d2, x);
    }

    DWORD WINAPI XInputGetKeystroke(DWORD d1, DWORD d2, XINPUT_KEYSTROKE* x)
    {
        init();
        if (!_XInputGetKeystrokeProc)
            return ERROR;

        return _XInputGetKeystrokeProc(d1, d2, x);
    }

    DWORD WINAPI XInputGetState(DWORD d1, XINPUT_STATE* x)
    {
        init();
        if (!_XInputGetStateProc)
            return ERROR;

        return _XInputGetStateProc(d1, x);
    }

    DWORD WINAPI XInputSetState(DWORD d1, XINPUT_VIBRATION* x)
    {
        init();
        if (!_XInputSetStateProc)
            return ERROR;

        return _XInputSetStateProc(d1, x);
    }
}
