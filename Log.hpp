#pragma once

#include <string>

namespace AutomataMod {

enum struct LogLevel
{
    LOG_INFO,
    LOG_ERROR
};

void log(LogLevel level, const char* message);
void log(LogLevel level, const std::string& message);
void log(LogLevel level, const std::wstring& message);

// Should only be used for unrecoverable errors we want to notify the user about
void showErrorBox(const char* message);

} // namespace AutomataMod