#include "DLLHook.hpp"

#include <vector>

DLLHook::DLLHook(const std::string &moduleName) {
	m_module = NULL;

	AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Loading {}", moduleName);
	std::vector<CHAR> buff(1024);
	UINT len = GetSystemWindowsDirectory(buff.data(), buff.size());
	if (len > buff.size()) {
		buff.resize(len);
		len = GetSystemWindowsDirectory(buff.data(), len);
	}

	if (len == 0) {
		// failed to get system directory
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get windows system directory. Error code: {}",
										 GetLastError());
		return;
	}

	std::string filePath(buff.begin(), buff.begin() + len);
	filePath += "\\system32\\" + moduleName;
	AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Attempting to load {}", filePath);
	m_module = LoadLibrary(filePath.c_str());
	if (m_module == NULL) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to load {} Error code: {}", moduleName, GetLastError());
	}

	AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "Loaded {}", moduleName);
}

DLLHook ::~DLLHook() {
	if (m_module) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "DLLHook is dropping loaded DLL");
		FreeLibrary(m_module);
		m_module = NULL;
	}
}

bool DLLHook::isModuleFound() const { return m_module != NULL; }
