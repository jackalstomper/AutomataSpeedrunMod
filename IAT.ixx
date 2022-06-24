module;

#include <Windows.h>
#include <stdint.h>

export module IAT;

import Log;

namespace IAT {

export class IATHook {
    PIMAGE_THUNK_DATA m_thunkIAT;
    ULONGLONG m_originalFunction;

    void parseImports(uint64_t baseAddress, const char* moduleName, const char* functionName, LPCVOID replacementFunction) {
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to parse dos header");
            return;
        }

        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(baseAddress + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to parse NT header");
            return;
        }

        IMAGE_OPTIONAL_HEADER optionalHeader = ntHeaders->OptionalHeader;
        if (optionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to parse NT optional header");
            return;
        }

        PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(baseAddress + optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        for (int i = 0; importDescriptor[i].Characteristics != 0; ++i) {
            PCHAR dllName = (PCHAR)(baseAddress + importDescriptor[i].Name);
            if (dllName != nullptr) {
                if (strcmp(moduleName, dllName) == 0) {
                    readImportDescriptor(importDescriptor[i], baseAddress, functionName, replacementFunction);
                    return;
                }
            }
        }

        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to hook {} from module {}", functionName, moduleName);
    }

    void readImportDescriptor(IMAGE_IMPORT_DESCRIPTOR& importDescriptor, uint64_t baseAddress, const char* functionName, LPCVOID replacementFunction) {
        if (importDescriptor.OriginalFirstThunk == 0) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get OriginalFirstThunk");
            return;
        }

        if (importDescriptor.FirstThunk == 0) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get FirstThunk");
            return;
        }

        PIMAGE_THUNK_DATA thunkILT = (PIMAGE_THUNK_DATA)(baseAddress + importDescriptor.OriginalFirstThunk);
        m_thunkIAT = (PIMAGE_THUNK_DATA)(baseAddress + importDescriptor.FirstThunk);

        PIMAGE_IMPORT_BY_NAME nameData;
        while (thunkILT->u1.AddressOfData != 0) {
            if (!(thunkILT->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                nameData = (PIMAGE_IMPORT_BY_NAME)(baseAddress + thunkILT->u1.AddressOfData);
                if (strcmp(functionName, nameData->Name) == 0) {
                    DWORD oldPermissions;
                    VirtualProtect(&(m_thunkIAT->u1.Function), sizeof(ULONGLONG), PAGE_EXECUTE_READWRITE, &oldPermissions);
                    m_originalFunction = m_thunkIAT->u1.Function;
                    m_thunkIAT->u1.Function = (ULONGLONG)replacementFunction;
                    VirtualProtect(&(m_thunkIAT->u1.Function), sizeof(ULONGLONG), oldPermissions, nullptr);
                    AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Successfully hooked {}", functionName);
                    return;
                }
            }

            thunkILT++;
            m_thunkIAT++;
        }

        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to hook function {}", functionName);
    }

public:
    IATHook(const char* moduleName, const char* functionName, LPCVOID replacementFunction) {
        m_thunkIAT = nullptr;
        m_originalFunction = 0;
        parseImports(reinterpret_cast<uint64_t>(GetModuleHandle(nullptr)), moduleName, functionName, replacementFunction);
    }

    ~IATHook() {
        if (m_thunkIAT && m_originalFunction) {
            DWORD oldPermissions;
            VirtualProtect(&(m_thunkIAT->u1.Function), sizeof(ULONGLONG), PAGE_EXECUTE_READWRITE, &oldPermissions);
            m_thunkIAT->u1.Function = m_originalFunction;
            VirtualProtect(&(m_thunkIAT->u1.Function), sizeof(ULONGLONG), oldPermissions, nullptr);
        }

        m_thunkIAT = nullptr;
        m_originalFunction = 0;
    }
};

} // namespace IAT