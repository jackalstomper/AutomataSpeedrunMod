#pragma once

#include <string>
#include <Windows.h>
#include "Log.hpp"

class DLLHook {
    HMODULE m_module;

public:
    DLLHook() : m_module(NULL) {}
    DLLHook(const std::string& moduleName);
    ~DLLHook();

    template<typename FuncPtr>
    FuncPtr hookFunc(const std::string& funcName) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Hooking " + funcName);
        if (!m_module) {
            return nullptr;
        }

        AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Hooked " + funcName);
        FuncPtr func = (FuncPtr)GetProcAddress(m_module, funcName.c_str());
        if (!func) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to hook " + funcName);
            return nullptr;
        }

        return func;
    }

    operator HMODULE() const;
};