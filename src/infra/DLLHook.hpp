#pragma once

#include "infra/Log.hpp"
#include <Windows.h>
#include <string>

class DLLHook {
	HMODULE m_module;

public:
	DLLHook(const std::string &moduleName);
	~DLLHook();
	bool isModuleFound() const;

	template <typename FuncPtr> FuncPtr hookFunc(const std::string &funcName) {
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
};
