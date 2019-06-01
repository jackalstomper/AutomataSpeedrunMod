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

} // namespace AutomataMod