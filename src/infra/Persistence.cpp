#include "Log.hpp"
#include <Windows.h>
#include <string>
#include <vector>

namespace {
const char *ENABLED_KEY_PATH = "Software\\AutomataSpeedrunMod";
const char *ENABLED_VALUE_NAME = "ModEnabled";

// Wrapper for HKEY to provide automatic closing to any open registry keys
struct RegKey {
	HKEY key;
	RegKey() : key(nullptr) {}
	~RegKey() {
		if (key != nullptr) {
			RegCloseKey(key);
			key = nullptr;
		}
	}
};

std::string getWindowsErrorString(LSTATUS status) {
	std::vector<char> buff(1024);
	DWORD count = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, status, 0, buff.data(), buff.size(), NULL);
	if (count == 0) {
		return fmt::format("{}", status);
	}
	std::string out(buff.begin(), buff.begin() + count);
	return fmt::format("Error Code: {}, Error Message: {}", status, out);
}

} // namespace

namespace Persistence {

void setEnabledFlag(bool enabled) {
	DWORD value = enabled;
	AutomataMod::log(AutomataMod::LogLevel::LOG_DEBUG, "Writing persistant flag to {}", value);
	RegKey regKey;
	LSTATUS status = RegCreateKeyExA(
			HKEY_CURRENT_USER, ENABLED_KEY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regKey.key, NULL
	);
	if (status != ERROR_SUCCESS) {
		AutomataMod::log(
				AutomataMod::LogLevel::LOG_ERROR, "Failed to open persistent enabled flag. {}", getWindowsErrorString(status)
		);
		return;
	}
	status = RegSetValueExA(regKey.key, ENABLED_VALUE_NAME, 0, REG_DWORD, (BYTE *)&value, sizeof(DWORD));
	if (status == ERROR_SUCCESS) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_DEBUG, "Persistant flag written to registry");
	} else {
		AutomataMod::log(
				AutomataMod::LogLevel::LOG_ERROR, "Failed to set persistent enabled flag. {}", getWindowsErrorString(status)
		);
	}
}

bool getEnabledFlag() {
	AutomataMod::log(AutomataMod::LogLevel::LOG_DEBUG, "Reading persistant flag");
	DWORD value;
	DWORD valueSize = sizeof(DWORD);
	LSTATUS status =
			RegGetValueA(HKEY_CURRENT_USER, ENABLED_KEY_PATH, ENABLED_VALUE_NAME, RRF_RT_REG_DWORD, NULL, &value, &valueSize);
	if (status == ERROR_SUCCESS) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_DEBUG, "Persistant flag read: {}", value);
		return value == 1;
	}
	if (status == ERROR_FILE_NOT_FOUND) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_INFO, "No persistent enabled flag found. Defaulting to off.");
		setEnabledFlag(false);
	} else {
		AutomataMod::log(
				AutomataMod::LogLevel::LOG_ERROR, "Failed to open persistent enabled flag. {}", getWindowsErrorString(status)
		);
	}
	return false;
}

} // namespace Persistence
