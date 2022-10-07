#pragma once

#include <fmt/format.h>
#include <string>

namespace AutomataMod {

enum struct LogLevel { LOG_INFO, LOG_ERROR, LOG_DEBUG };

/**
 * @brief Log function that supports FMT formatting
 * @param level The log level to use
 * @param formatString The FMT format string to use
 * @param ...args Format string values
 */
template <typename... T> void log(LogLevel level, const std::string_view &fmt, T &&...args) {
  std::string string = fmt::format(fmt::runtime(fmt), args...);
  log(level, string.c_str());
}

void log(LogLevel level, const char *message);

} // namespace AutomataMod