#pragma once

#include <string>
#include <fmt/format.h>

namespace AutomataMod {

enum struct LogLevel {
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
template<typename... T>
void logEx(LogLevel level, fmt::format_string<T...> formatString, T&&... args) {
    std::string string = fmt::format(formatString, args...);
    log(level, string);
}

void log(LogLevel level, const char* message);
void log(LogLevel level, const std::string& message);
void log(LogLevel level, const std::wstring& message);

// Should only be used for unrecoverable errors we want to notify the user about
void showErrorBox(const char* message);

} // namespace AutomataMod