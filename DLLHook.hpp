#pragma once

#include <string>
#include <Windows.h>
#include "Log.hpp"

class DLLHook {
    HMODULE m_module;

public:
    DLLHook(const std::string& moduleName);
    ~DLLHook();

    template<typename FuncPtr>
    FuncPtr hookFunc(const std::string& funcName) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Hooking {}", funcName);
        if (!m_module) {
            return nullptr;
        }

        FuncPtr func = (FuncPtr)GetProcAddress(m_module, funcName.c_str());
        if (!func) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to hook {} Error code: {}", funcName, GetLastError());
            return nullptr;
        }

        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Hooked {}", funcName);
        return func;
    }

    bool isModuleFound() const;
};