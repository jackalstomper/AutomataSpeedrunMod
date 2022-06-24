module;

#include <windows.h>
#include <ShlObj.h>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <locale>
#include <vector>
#include <fmt/format.h>
#include <fmt/chrono.h>

export module Log;

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

namespace AutomataMod {

export enum struct LogLevel {
    LOG_INFO,
    LOG_ERROR,
    LOG_DEBUG
};

/**
 * @brief Log function that supports FMT formatting
 * @param level The log level to use
 * @param formatString The FMT format string to use
 * @param ...args Format string values
*/
export template<typename... T>
void log(LogLevel level, const std::string_view& fmt, T&&... args) {
    std::string string = fmt::format(fmt::runtime(fmt), args...);
    log(level, string.c_str());
}

export void log(LogLevel level, const char* message) {
    initLog();
    if (!logFile.is_open())
        return;
    
    const char* levelString;
    if (level == LogLevel::LOG_ERROR) {
        levelString = "[ERROR]";
    } else if (level == LogLevel::LOG_INFO) {
        levelString = "[INFO]";
    } else {
        levelString = "[DEBUG]";
    }

    std::string line = fmt::format("{:%F %T} {} {}", fmt::localtime(std::time(nullptr)), levelString, message);
    logFile << line << std::endl;
}

} // namespace AutomataMod