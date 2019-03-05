// Proxy DLL for DirectInput 8 to provide an entry point for our code to modify automata with
#include <windows.h>
#include <psapi.h>
#include <thread>
#include <chrono>
#include "InventoryManager.hpp"

// Addresses are offsets in bytes relevant to processRamStart
const uint64_t CURRENT_PHASE_ADDR = 0x1101D58;
const uint64_t PLAYER_SET_NAME_ADDR = 0x147B4BC;
const uint64_t PLAYER_NAME_ADDR = 0x194BF88;
const uint64_t IS_WORLD_LOADED_ADDR = 0x110ADC0;
const uint64_t ITEM_TABLE_START_ADDR = 0x197C4C4;

const char16_t* MOD_NAME_POSTFIX = u"|AM";
const int MOD_NAME_POSTFIX_LEN = 3;

// Function pointer types for the DLL functions we have to proxy
typedef HRESULT(WINAPI *DirectInput8CreateProc)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HRESULT(WINAPI *DllGetClassObjectProc)(REFCLSID, REFIID, LPVOID *);
typedef HRESULT(WINAPI *DllCanUnloadNowProc)();
typedef HRESULT(WINAPI *DllRegisterServerProc)();
typedef HRESULT(WINAPI *DllUnregisterServerProc)();

DllGetClassObjectProc dllGetClassObjectFunc = nullptr;
DllCanUnloadNowProc dllCanUnloadNowFunc = nullptr;
DirectInput8CreateProc directInput8CreateFunc = nullptr;
DllRegisterServerProc dllRegisterServerFunc = nullptr;
DllUnregisterServerProc dllUnregisterServerFunc = nullptr;

HMODULE dInput8 = nullptr;
HMODULE processRamStart = nullptr;
std::thread* checkerThread = nullptr;

// Adds a signature onto the players savegame name to indicate the mod is in use
bool addPostfixToFilename(char16_t* playerName)
{
    int nameLen = 0;
    for (char16_t* i = playerName; i < playerName + 16; ++i, ++nameLen) {
        if (*i == 0)
            break;
    }

    if (nameLen == 0) // Something has gone wrong
        return false;

    char16_t* insertPoint;
    if (nameLen + MOD_NAME_POSTFIX_LEN > 16) {
        // Need to trim the name to fit our postfix
        insertPoint = playerName + nameLen - 3;
    } else {
        insertPoint = playerName + nameLen;
    }

    memcpy(insertPoint, MOD_NAME_POSTFIX, MOD_NAME_POSTFIX_LEN * sizeof(char16_t));
    return true;
}

// Our offsets are in bytes, so we need to compute the pointer using a byte sized type before converting into the type we desire
template<typename T>
T getOffsetPointer(uint64_t address)
{
    return reinterpret_cast<T>(reinterpret_cast<uint8_t*>(processRamStart) + address);
}

void checkStuff()
{
    char* currentPhase = getOffsetPointer<char*>(CURRENT_PHASE_ADDR);
    uint32_t* playerNameSet = getOffsetPointer<uint32_t*>(PLAYER_SET_NAME_ADDR);
    char16_t* playerName = getOffsetPointer<char16_t*>(PLAYER_NAME_ADDR);
    uint32_t* isWorldLoaded = getOffsetPointer<uint32_t*>(IS_WORLD_LOADED_ADDR);
    uint32_t* itemTableRamStart = getOffsetPointer<uint32_t*>(ITEM_TABLE_START_ADDR);
    bool modNameAdded = false;
    bool inventoryModded = false;

    while (true) {
        if (!modNameAdded && *isWorldLoaded == 1 && *playerNameSet == 1) {
            modNameAdded = addPostfixToFilename(playerName);
        }

        if (!inventoryModded && *isWorldLoaded == 1 && *playerNameSet == 1 && strncmp(currentPhase, "58_AB_BossArea_Fall", 19) == 0) {
            AutomataMod::InventoryManager manager(itemTableRamStart);
            manager.setVc3Inventory();
            inventoryModded = true;
        }

        if (*isWorldLoaded == 0 && *playerNameSet == 0) {
            if (modNameAdded)
                modNameAdded = false;

            if (inventoryModded)
                inventoryModded = false;
        }

        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(250)); // check stuff 4 times a second
    }
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
    directInput8CreateFunc = (DirectInput8CreateProc)GetProcAddress(dInput8, "DirectInput8Create");
    dllGetClassObjectFunc = (DllGetClassObjectProc)GetProcAddress(dInput8, "DllGetClassObject");
    dllCanUnloadNowFunc = (DllCanUnloadNowProc)GetProcAddress(dInput8, "DllCanUnloadNow");
    dllRegisterServerFunc = (DllRegisterServerProc)GetProcAddress(dInput8, "DllRegisterServer");
    dllUnregisterServerFunc = (DllUnregisterServerProc)GetProcAddress(dInput8, "DllUnregisterServer");
    checkerThread = new std::thread(checkStuff);
}

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

    HRESULT CALLBACK DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
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
