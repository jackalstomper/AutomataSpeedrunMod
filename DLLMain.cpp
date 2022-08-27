#include <windows.h>
#include <atlbase.h>
#include <ntstatus.h>
#include <bcrypt.h>
#include <Psapi.h>
#include <thread>
#include <vector>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <string>

#pragma comment(lib, "bcrypt.lib")

import AutomataMod;
import Log;
import IAT;
import FactoryWrapper;
import DLLHook;
import ModConfig;

std::unique_ptr<DLLHook> xinput;
std::unique_ptr<std::thread> checkerThread;
std::unique_ptr<IAT::IATHook> d3dCreateDeviceHook;
std::unique_ptr<IAT::IATHook> dxgiCreateFactoryHook;
std::unique_ptr<AutomataMod::ModChecker> modChecker;
CComPtr<DxWrappers::DXGIFactoryWrapper> wrapper;
bool shouldStopChecker = false;

enum NierVersion
{
    NIERVER_UNKNOWN,
    NIERVER_100,
    NIERVER_101,
    NIERVER_102,
    NIERVER_102_UNPACKED,
    NIERVER_WINSTORE,
    NIERVER_DEBUG,
    NIERVER_MAX
};

static std::ostream& operator<<(std::ostream& os, const NierVersion& v)
{
    switch (v)
    {
    case NIERVER_100:
        os << "NieR:Automata (v1.00)";
        break;
    case NIERVER_101:
        os << "NieR:Automata (v1.01)";
        break;
    case NIERVER_102:
        os << "NieR:Automata (v1.02)";
        break;
    case NIERVER_102_UNPACKED:
        os << "NieR:Automata (v1.02 Unpacked)";
        break;
    case NIERVER_WINSTORE:
        os << "NieR:Automata (Winstore)";
        break;
    case NIERVER_DEBUG:
        os << "NieR:Automata (Debug)";
        break;
    case NIERVER_UNKNOWN:
    default:
        os << "NieR:Automata (Unknown Version)";
        break;
    }
    return os;
}

// Author: Martino
struct NierVersionHash
{
    NierVersionHash(NierVersion Version)
        : m_pHash(nullptr), m_uHashSize(0), m_Version(Version)
    {
    }

    // C++ 20 kekw
    NierVersionHash(NierVersion Version, const char* Hash)
        : m_pHash(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(Hash))),
        m_uHashSize(strlen(Hash)), m_Version(Version)
    {
    }

    bool operator==(NierVersionHash& other) const
    {
        return !memcmp(this->m_pHash, other.m_pHash, m_uHashSize);
    }

    bool operator==(const uint8_t* pHash) const
    {
        return !memcmp(this->m_pHash, pHash, m_uHashSize);
    }

    uint8_t* m_pHash;
    uint32_t m_uHashSize;
    NierVersion m_Version;
};

// Author: Martino

/// <summary>
/// Hashes the file of the currently loaded NieR:Automata binary (SHA-256)
/// </summary>
/// <param name="pHash">A reference to a uint8_t pointer to strore the SHA-256 hash </param>
/// <param name="uHashSize">A reference to uint32_t to be set to the hash size in bytes</param>
/// <returns>STATUS_SUCCESS on success (use the FAILED and SUCCEEDED marcos).</returns>
NTSTATUS QueryhNierBinaryHash(uint8_t*& pHash, uint32_t& uHashSize)
{
    TCHAR szFileName[MAX_PATH];

    // Set the inital status to success
    NTSTATUS Status = STATUS_SUCCESS;

    // Query the nier binary file path
    GetModuleFileName(nullptr, szFileName, ARRAYSIZE(szFileName));

    // Query a file handle to the nier binary 
    HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    // If the handle is invalid, bail
    if (hFile == INVALID_HANDLE_VALUE)
        return STATUS_INVALID_HANDLE;

    // Query the file size of the nier binary
    uint32_t uFileSize = GetFileSize(hFile, nullptr);
    uint32_t uBytesRead;

    // Allocate the memory for the nier binary
    uint8_t* pBinary = new uint8_t[uFileSize];

    // Read the binary file
    ReadFile(hFile, pBinary, uFileSize, (PDWORD)&uBytesRead, nullptr);

    // Close the file handle to the nier binary
    CloseHandle(hFile);

    // Bail out if there was a partial read
    if (uFileSize != uFileSize)
    {
        delete[] pBinary;
        return ERROR_CLUSTER_PARTIAL_READ;
    }

    uint32_t uHashLengthSize;

    // Query the hash length
    Status = BCryptGetProperty(BCRYPT_SHA256_ALG_HANDLE, BCRYPT_HASH_LENGTH, (PUCHAR)&uHashSize, sizeof(uint32_t), (PULONG)&uHashLengthSize, 0);

    if (SUCCEEDED(Status))
    {
        // Allocate the memory for hash
        pHash = new uint8_t[uHashSize];

        // Create the hash
        Status = BCryptHash(BCRYPT_SHA256_ALG_HANDLE, nullptr, 0, pBinary, uFileSize, pHash, uHashSize);
    }

    // Free the binary from memory
    delete[] pBinary;

    return Status;
}

/// Author: Martino
/// <summary>
/// Queries the currently loaded NieR:Automata binary version
/// </summary>
/// <returns>the binary version of the currently loaded NieR:Automata binary</returns>
NierVersion QueryNierBinaryVersion(void)
{
    static NierVersionHash Versions[] =
    {
        { NIERVER_101, "\xa0\x1a\xc5\x13\x2e\x10\x92\x52\xd6\xd9\xa4\xcb\xf9\x74\x61\x4d\xec\xfb\xe3\x23\x71\x3c\x1f\xbf\x5b\xc2\x48\xf0\x12\x61\x77\x3f" },
        { NIERVER_102, "\x51\x71\xbe\xd0\x9e\x6f\xec\x7b\x21\xbf\x0e\xa4\x79\xdb\xd2\xe1\xb2\x28\x69\x5c\x67\xd1\xf0\xb4\x78\x54\x9a\x9b\xe2\xf5\x72\x6a" },
        { NIERVER_102_UNPACKED, "\x5f\x97\x20\xd8\xc7\x7c\xd5\x97\x8e\xfe\x49\x88\x89\x3a\xf8\xfd\x99\x9f\x90\xa4\x76\xa8\xde\xeb\xb3\x91\x26\x94\xf6\x18\xdc\x43" },
        { NIERVER_WINSTORE, "\x3d\xde\x56\x6c\xea\x3e\x3b\xc1\x5e\x45\x92\x66\x02\xfb\x4f\x24\xd4\x8f\x77\xdf\x8a\x7b\xc5\x50\xa5\xb2\xdc\xae\xcc\xcf\x09\x48" },
        { NIERVER_DEBUG, "\xe9\xef\x66\x01\xeb\x40\xeb\x0a\x6d\x3f\x30\xa6\x63\x95\x43\xec\x2f\x81\x71\xc2\x6a\x3d\xe8\xb2\xb1\x30\x39\xee\xbe\x3b\xc8\x1c" },
        { NIERVER_MAX }
    };
    uint8_t* pHash;
    uint32_t uHashSize;

    // Set the version to unknown at first
    NierVersion Version = NIERVER_UNKNOWN;

    // Query the nier binary hash
    if (FAILED(QueryhNierBinaryHash(pHash, uHashSize)))
        return NIERVER_UNKNOWN;

    // Traverse the array comparing the loaded binary hash to the known hashes
    for (NierVersionHash* pVer = Versions; pVer->m_Version != NIERVER_MAX && Version == NIERVER_UNKNOWN; ++pVer)
        if (*pVer == pHash)
            Version = pVer->m_Version;

    // Free the hash memory
    delete[] pHash;

    return Version;
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
    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Process ram start: 0x{:X}", processRamStartAddr);

    AutomataMod::ModConfig modConfig;
    NierVersion Version = QueryNierBinaryVersion();

    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Detected ", Version);

    if (Version == NIERVER_102 || Version == NIERVER_102_UNPACKED) {      
        AutomataMod::Addresses addresses;
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
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Could not determine what automata version we are. VC3Mod will not boot. moduleSize: {}", Version);
        return;
    }

    modChecker = std::make_unique<AutomataMod::ModChecker>(processRamStartAddr, std::move(modConfig));

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
