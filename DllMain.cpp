#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <vector>
#include "AutomataMod.hpp"
#include "Log.hpp"

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
uint64_t processRamStartAddr = 0;

template<typename T>
T getProc(const char* procName)
{
    return reinterpret_cast<T>(GetProcAddress(hProxyHandle, procName));
}

void tick()
{
    if (processRamStartAddr != 0)
        AutomataMod::checkStuff(processRamStartAddr);
}

void init()
{
    if (hProxyHandle)
        return;

    using LogLevel = AutomataMod::LogLevel;
    AutomataMod::log(LogLevel::LOG_INFO, "Initializing AutomataMod");

    // Get the starting memory address for automata
    HANDLE currentProcess = GetCurrentProcess();
    HMODULE hMod;
    DWORD cbNeeded;

    if (!EnumProcessModulesEx(currentProcess, &hMod, sizeof(hMod), &cbNeeded, LIST_MODULES_64BIT)) {
        AutomataMod::log(LogLevel::LOG_ERROR, "Failed to find process ram start for automata");
        return;
    }

    processRamStart = hMod;
    processRamStartAddr = reinterpret_cast<uint64_t>(processRamStart);
    AutomataMod::log(LogLevel::LOG_INFO, "Found process ram start: " + std::to_string(processRamStartAddr));

    // Get windows directory for this system so we can load the real library
    AutomataMod::log(LogLevel::LOG_INFO, "Trying to find system directory to load real xinput DLL");
    std::vector<WCHAR> buff(1024);
    UINT dirLen = GetSystemDirectoryW(buff.data(), 1024);
    if (dirLen == 0) {
        AutomataMod::log(LogLevel::LOG_ERROR, "Failed to find system directory to load real DLL");
        return;
    }

    if (dirLen + 15 > 1024) {
        // windows directory is too long for our buffer, use a dynamic buffer
        // when GetSystemDirectory fails dirLen will be size of buffer needed including null terminating character
        buff = std::vector<WCHAR>(dirLen + 15);
        dirLen = GetSystemDirectoryW(buff.data(), dirLen);
    }

    std::wstring libPath(buff.data(), dirLen);
    libPath += L"\\xinput1_3.dll";
    AutomataMod::log(LogLevel::LOG_INFO, L"Found directory: " + libPath);

    // Append the dll directory to windows dir
    hProxyHandle = LoadLibraryExW(libPath.c_str(), NULL, 0);
    if (!hProxyHandle) {
        AutomataMod::log(LogLevel::LOG_ERROR, "Failed to load real xinput DLL");
        return;
    }

    // Get addresses to the real functions so we can forward our calls to them
    AutomataMod::log(LogLevel::LOG_INFO, "Assigning procs");
    _XInputEnableProc = getProc<XInputEnableProc>("XInputEnable");
    _XInputGetBatteryInformationProc = getProc<XInputGetBatteryInformationProc>("XInputGetBatteryInformation");
    _XInputGetCapabilitiesProc = getProc<XInputGetCapabilitiesProc>("XInputGetCapabilities");
    _XInputGetKeystrokeProc = getProc<XInputGetKeystrokeProc>("XInputGetKeystroke");
    _XInputGetStateProc = getProc<XInputGetStateProc>("XInputGetState");
    _XInputSetStateProc = getProc<XInputSetStateProc>("XInputSetState");
}

} // namespace

extern "C" {
    BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
    {
        if (reason == DLL_PROCESS_DETACH) {
            if (hProxyHandle) {
                FreeLibrary(hProxyHandle);
                hProxyHandle = nullptr;
                processRamStartAddr = 0;
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
        if (d1 == 0)
            tick();

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
