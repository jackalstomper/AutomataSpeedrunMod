#include <string>

namespace {
const std::string VC3_VERSION("1.10");
const std::wstring VC3_WVERSION(L"1.10");
} // namespace

namespace AutomataMod {
namespace Constants {

const std::string &getVersion() { return VC3_VERSION; }
const std::wstring &getWVersion() { return VC3_WVERSION; }

} // namespace Constants
} // namespace AutomataMod
