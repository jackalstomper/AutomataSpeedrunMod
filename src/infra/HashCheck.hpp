#pragma once

#include <string>

namespace AutomataMod {

enum NierVersion { NIERVER_UNKNOWN, NIERVER_101, NIERVER_102, NIERVER_102_UNPACKED, NIERVER_WINSTORE, NIERVER_DEBUG };

class NierVerisonInfo {
	NierVersion m_version;
	std::string_view m_versionName;

public:
	NierVerisonInfo();

	NierVerisonInfo(NierVersion version, std::string_view verisonName);

	NierVersion version() const;

	std::string_view versionName() const;

	bool operator==(NierVersion other) const;
};

/// Author: Martino
/// <summary>
/// Queries the currently loaded NieR:Automata binary version
/// </summary>
/// <throws>Throws std::runtime_error on any failure condition</throws>
/// <returns>the binary version of the currently loaded NieR:Automata binary</returns>
NierVerisonInfo QueryNierBinaryVersion();

} // namespace AutomataMod
