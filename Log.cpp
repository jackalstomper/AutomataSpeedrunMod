#include "Log.hpp"

#ifdef AUTOMATA_LOG

#include <windows.h>
#include <ShlObj.h>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <locale>
#include <codecvt>

#ifndef AUTOMATA_RELEASE_TARGET
#include "DxWrappers.hpp"
extern DxWrappers::DXGIFactoryWrapper* g_wrapper;
#endif

namespace {

LPCWSTR logFolderName = L".automataMod";
std::ofstream logFile;

void initLog()
{
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

} // namespace

#endif

namespace AutomataMod {


void log(LogLevel level, const std::string& message)
{
#ifdef AUTOMATA_LOG
    log(level, message.c_str());
#endif
}


void log(LogLevel level, const std::wstring& message)
{
#ifdef AUTOMATA_LOG
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    log(level, converter.to_bytes(message));
#endif
}

void log(LogLevel level, const char* message)
{
#ifdef AUTOMATA_LOG
    initLog();
    if (!logFile.is_open())
        return;

    std::time_t t = std::time(nullptr);
    std::tm tm;
    errno_t result = localtime_s(&tm, &t);
    if (result == 0)
        logFile << std::put_time(&tm, "%F %T") << ' ';

    if (level == LogLevel::LOG_ERROR) {
        logFile << "[ERROR] ";
    } else {
        logFile << "[INFO] ";
    }

    logFile << message << std::endl;

#ifndef AUTOMATA_RELEASE_TARGET
    if (g_wrapper) // Write onscreen log
        g_wrapper->writeLog(message);
#endif // AUTOMATA_RELEASE_TARGET
#endif // AUTOMATA_LOG
}

void showErrorBox(const char* message) {
    MessageBox(NULL, message, "AutomataMod has encountered an Error", MB_OK | MB_ICONSTOP);
}

} // namespace AutomataMod