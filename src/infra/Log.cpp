#include "Log.hpp"

#include <ShlObj.h>
#include <ctime>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fstream>
#include <iomanip>
#include <locale>
#include <vector>
#include <windows.h>

#ifdef _DEBUG
#include <cstdio>
#endif

namespace {

LPCWSTR logFolderName = L".automataMod";
std::ofstream logFile;

void initLog() {
	if (logFile.is_open())
		return;

	PWSTR roamingPath = NULL; // The folder path once SHGetKnownFolderPath returns. We're responsible for freeing this.
	HRESULT result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &roamingPath);
	if (result != S_OK) {
		CoTaskMemFree(roamingPath);
		return; // We failed to get the roaming folder and we haven't init our own log so not much we can do here
	}

	std::wstring logFolder(roamingPath);
	CoTaskMemFree(roamingPath);

	logFolder += L'\\';
	logFolder += logFolderName;

	WIN32_FIND_DATAW findData;
	HANDLE folder = FindFirstFileW(logFolder.c_str(), &findData);
	if (folder == INVALID_HANDLE_VALUE) {
		// Our log folder doesn't exist. Lets make it.
		BOOL result = CreateDirectoryW(logFolder.c_str(), NULL);
		if (!result)
			return; // We couldn't create our log folder. Give up.
	}

	logFolder += L"\\automataMod.log";
	logFile.open(logFolder, std::ios::out | std::ios::trunc);
}

#ifdef _DEBUG
void openConsole() {
	static bool consoleOpened = false;
	if (consoleOpened)
		return;
	consoleOpened = AllocConsole();
}

void logToConsole(const std::string &str) {
	static HANDLE stdOutHandle = nullptr;
	openConsole();
	if (!stdOutHandle)
		stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (!stdOutHandle)
		return;

	WriteConsole(stdOutHandle, str.c_str(), str.size(), nullptr, nullptr);
}
#endif

} // namespace

namespace AutomataMod {

void log_raw(LogLevel level, const char *message) {
#ifndef _DEBUG
	initLog();
	if (!logFile.is_open())
		return;

	if (level == LogLevel::LOG_DEBUG)
		return;
#endif

	const char *levelString;
	if (level == LogLevel::LOG_ERROR) {
		levelString = "[ERROR]";
	} else if (level == LogLevel::LOG_INFO) {
		levelString = "[INFO]";
	} else {
		levelString = "[DEBUG]";
	}

	std::string line = fmt::format("{:%F %T} {} {}", fmt::localtime(std::time(nullptr)), levelString, message);

#if defined(_DEBUG)
	line += '\n';
	OutputDebugString(line.c_str());
	logToConsole(line);
#else
	logFile << line << std::endl;
#endif
}

} // namespace AutomataMod
