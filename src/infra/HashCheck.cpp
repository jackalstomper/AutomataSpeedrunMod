#include "HashCheck.hpp"
#include "infra/defs.hpp"

#ifdef _DEBUG
#include "infra/Log.hpp"
#include <fmt/format.h>
#endif

#include <Windows.h>
#include <bcrypt.h>
#include <stdexcept>
#include <unordered_map>

namespace {

using namespace AutomataMod;

const std::unordered_map<std::string, NierVerisonInfo> VERSION_HASHES = {
		{"\xa0\x1a\xc5\x13\x2e\x10\x92\x52\xd6\xd9\xa4\xcb\xf9\x74\x61\x4d\xec\xfb\xe3\x23\x71\x3c\x1f\xbf\x5b\xc2\x48\xf0"
		 "\x12\x61\x77\x3f",
		 {NierVersion::NIERVER_101, "NieR:Automata (v1.01)"}},
		{"\x51\x71\xbe\xd0\x9e\x6f\xec\x7b\x21\xbf\x0e\xa4\x79\xdb\xd2\xe1\xb2\x28\x69\x5c\x67\xd1\xf0\xb4\x78\x54\x9a\x9b"
		 "\xe2\xf5\x72\x6a",
		 {NierVersion::NIERVER_102, "NieR:Automata (v1.02)"}},
		{"\x3d\xde\x56\x6c\xea\x3e\x3b\xc1\x5e\x45\x92\x66\x02\xfb\x4f\x24\xd4\x8f\x77\xdf\x8a\x7b\xc5\x50\xa5\xb2\xdc\xae"
		 "\xcc\xcf\x09\x48",
		 {NierVersion::NIERVER_WINSTORE, "NieR:Automata (Winstore)"}},
		{"\xe9\xef\x66\x01\xeb\x40\xeb\x0a\x6d\x3f\x30\xa6\x63\x95\x43\xec\x2f\x81\x71\xc2\x6a\x3d\xe8\xb2\xb1\x30\x39\xee"
		 "\xbe\x3b\xc8\x1c",
		 {NierVersion::NIERVER_DEBUG, "NieR:Automata (Debug)"}}};

std::string getNierFileName() {
	std::vector<char> pathBuffer(MAX_PATH);
	DWORD copied = 0;
	do {
		copied = GetModuleFileName(nullptr, pathBuffer.data(), pathBuffer.size());
		if (copied == 0) {
			DWORD error = GetLastError();
			if (!SUCCEEDED(error)) {
				if (error == ERROR_INSUFFICIENT_BUFFER) {
					pathBuffer.resize(pathBuffer.size() + MAX_PATH);
				} else {
					throw std::runtime_error("Failed to get module name for executable");
				}
			}
		}
	} while (copied >= pathBuffer.size());
	return std::string(pathBuffer.begin(), pathBuffer.end());
}

std::vector<u8> readFile(const std::string &fileName) {
	// Query a file handle to the nier binary
	HANDLE hFile = CreateFile(
			fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
	);

	// If the handle is invalid, bail
	if (hFile == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Attempted to load invalid executuable file for hash check");

	// Query the file size of the nier binary
	u32 fileSize = GetFileSize(hFile, nullptr);
	u32 bytesRead;

	// Allocate the memory for the nier binary
	std::vector<u8> fileBuff(fileSize);

	// Read the binary file
	ReadFile(hFile, fileBuff.data(), fileBuff.size(), (PDWORD)&bytesRead, nullptr);
	CloseHandle(hFile);

	// Bail out if there was a partial read
	if (fileSize != bytesRead) {
		throw std::runtime_error("Partial read while calculating executable hash");
	}

	return fileBuff;
}

std::string QueryNierBinaryHash() {
	std::string fileName = getNierFileName();
	std::vector<u8> fileBuff = readFile(fileName);

	// Query the hash length
	u32 hashSize;
	u32 hashLengthSize;
	NTSTATUS status = BCryptGetProperty(
			BCRYPT_SHA256_ALG_HANDLE, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&hashSize), sizeof(u32),
			reinterpret_cast<PULONG>(&hashLengthSize), 0
	);

	if (!SUCCEEDED(status)) {
		throw std::runtime_error("Failed to calculate hash for executable");
	}

	std::vector<u8> hash(hashSize);
	status = BCryptHash(BCRYPT_SHA256_ALG_HANDLE, nullptr, 0, fileBuff.data(), fileBuff.size(), hash.data(), hash.size());

	return std::string(hash.begin(), hash.end());
}

} // namespace

namespace AutomataMod {

NierVerisonInfo::NierVerisonInfo() : m_version(NIERVER_UNKNOWN), m_versionName("NieR:Automata (Unknown)") {}

NierVerisonInfo::NierVerisonInfo(NierVersion version, std::string_view verisonName)
		: m_version(version), m_versionName(verisonName) {}

NierVersion NierVerisonInfo::version() const { return m_version; }

std::string_view NierVerisonInfo::versionName() const { return m_versionName; }

bool NierVerisonInfo::operator==(NierVersion other) const { return m_version == other; }

std::optional<NierVerisonInfo> QueryNierBinaryVersion() {
	// Query the nier binary hash
	std::string exeHash = QueryNierBinaryHash();

#ifdef _DEBUG
	std::string out;
	for (char c : exeHash) {
		out += fmt::format("\\x{:x}", (unsigned char)c);
	}

	AutomataMod::log(AutomataMod::LogLevel::LOG_DEBUG, "Nier version hash: {}", out);
#endif

	auto it = VERSION_HASHES.find(exeHash);
	if (it != VERSION_HASHES.end()) {
		return it->second;
	}

	return {};
}

} // namespace AutomataMod
