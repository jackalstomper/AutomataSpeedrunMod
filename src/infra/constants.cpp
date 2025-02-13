#include "constants.hpp"
#include <string>

namespace {
const std::string VC3_VERSION("1.12");
const std::wstring VC3_WVERSION(L"1.12");
} // namespace

namespace AutomataMod {
namespace Constants {

const u32 INVALID_WINDOW_MODE = 99;

const std::string &getVersion() { return VC3_VERSION; }
const std::wstring &getWVersion() { return VC3_WVERSION; }

} // namespace Constants
} // namespace AutomataMod
