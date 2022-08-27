module;

#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <Windows.h>
#include <bcrypt.h>

export module HashCheck;

namespace AutomataMod {

export enum NierVersion {
    NIERVER_UNKNOWN,
    NIERVER_101,
    NIERVER_102,
    NIERVER_102_UNPACKED,
    NIERVER_WINSTORE,
    NIERVER_DEBUG
};

export class NierVerisonInfo {
    NierVersion m_version;
    std::string m_versionName;
    std::string m_hash;

public:
    NierVerisonInfo() :
        m_version(NIERVER_UNKNOWN),
        m_versionName("NieR:Automata (Unknown)")
    {}

    NierVerisonInfo(NierVersion version, std::string verisonName, std::string hash) :
        m_version(version),
        m_versionName(verisonName),
        m_hash(hash)
    {}

    NierVersion version() const {
        return m_version;
    }

    const std::string& versionName() const {
        return m_versionName;
    }

    const std::string& hash() const {
        return m_hash;
    }

    bool operator==(NierVersion other) {
        return m_version == other;
    }
};

const std::vector<NierVerisonInfo> VERSION_HASHES = {
    { NIERVER_101, "NieR:Automata (v1.01)", "\xa0\x1a\xc5\x13\x2e\x10\x92\x52\xd6\xd9\xa4\xcb\xf9\x74\x61\x4d\xec\xfb\xe3\x23\x71\x3c\x1f\xbf\x5b\xc2\x48\xf0\x12\x61\x77\x3f" },
    { NIERVER_102, "NieR:Automata (v1.02)", "\x51\x71\xbe\xd0\x9e\x6f\xec\x7b\x21\xbf\x0e\xa4\x79\xdb\xd2\xe1\xb2\x28\x69\x5c\x67\xd1\xf0\xb4\x78\x54\x9a\x9b\xe2\xf5\x72\x6a" },
    { NIERVER_102_UNPACKED, "NieR:Automata (v1.02 Unpacked)", "\x5f\x97\x20\xd8\xc7\x7c\xd5\x97\x8e\xfe\x49\x88\x89\x3a\xf8\xfd\x99\x9f\x90\xa4\x76\xa8\xde\xeb\xb3\x91\x26\x94\xf6\x18\xdc\x43" },
    { NIERVER_WINSTORE, "NieR:Automata (Winstore)", "\x3d\xde\x56\x6c\xea\x3e\x3b\xc1\x5e\x45\x92\x66\x02\xfb\x4f\x24\xd4\x8f\x77\xdf\x8a\x7b\xc5\x50\xa5\xb2\xdc\xae\xcc\xcf\x09\x48" },
    { NIERVER_DEBUG, "NieR:Automata (Debug)", "\xe9\xef\x66\x01\xeb\x40\xeb\x0a\x6d\x3f\x30\xa6\x63\x95\x43\xec\x2f\x81\x71\xc2\x6a\x3d\xe8\xb2\xb1\x30\x39\xee\xbe\x3b\xc8\x1c" }
};

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

std::vector<uint8_t> readFile(const std::string& fileName) {
    // Query a file handle to the nier binary
    HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    // If the handle is invalid, bail
    if (hFile == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Attempted to load invalid executuable file for hash check");

    // Query the file size of the nier binary
    uint32_t fileSize = GetFileSize(hFile, nullptr);
    uint32_t bytesRead;

    // Allocate the memory for the nier binary
    std::vector<uint8_t> fileBuff(fileSize);

    // Read the binary file
    ReadFile(hFile, fileBuff.data(), fileBuff.size(), (PDWORD)&bytesRead, nullptr);
    CloseHandle(hFile);

    // Bail out if there was a partial read
    if (fileSize != bytesRead) {
        throw std::runtime_error("Partial read while calculating executable hash");
    }

    return fileBuff;
}

// Author: Martino
/// <summary>
/// Hashes the file of the currently loaded NieR:Automata binary (SHA-256)
/// </summary>
/// <throws>Throws std::runtime_error on any failure condition</throws>
/// <returns>The calculated SHA256 hash of the automata executable</returns>
std::string QueryhNierBinaryHash() {
    std::string fileName = getNierFileName();
    std::vector<uint8_t> fileBuff = readFile(fileName);

    // Query the hash length
    uint32_t hashSize;
    uint32_t hashLengthSize;
    NTSTATUS status = BCryptGetProperty(
        BCRYPT_SHA256_ALG_HANDLE,
        BCRYPT_HASH_LENGTH,
        reinterpret_cast<PUCHAR>(&hashSize),
        sizeof(uint32_t),
        reinterpret_cast<PULONG>(&hashLengthSize),
        0);

    if (!SUCCEEDED(status)) {
        throw std::runtime_error("Failed to calculate hash for executable");
    }

    std::vector<uint8_t> hash(hashSize);
    status = BCryptHash(
        BCRYPT_SHA256_ALG_HANDLE,
        nullptr,
        0,
        fileBuff.data(),
        fileBuff.size(),
        hash.data(),
        hash.size());

    return std::string(hash.begin(), hash.end());
}

/// Author: Martino
/// <summary>
/// Queries the currently loaded NieR:Automata binary version
/// </summary>
/// <throws>Throws std::runtime_error on any failure condition</throws>
/// <returns>the binary version of the currently loaded NieR:Automata binary</returns>
export const NierVerisonInfo& QueryNierBinaryVersion() {
    // Query the nier binary hash
    std::string exeHash = QueryhNierBinaryHash();

    for (const auto& vHash : VERSION_HASHES) {
        if (vHash.hash() == exeHash) {
            return vHash;
        }
    }

    throw std::runtime_error("Failed to determine Nier verison from executable");
}

} // namespace AutomataMod