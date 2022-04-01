#pragma once

#include <cstdint>

namespace AutomataMod {

class ModConfig {
public:
    struct Addresses {
        uint64_t currentPhase;
        uint64_t isWorldLoaded;
        uint64_t playerSetName;
        uint64_t isLoading;
        uint64_t itemTableStart;
        uint64_t chipTableStart;
        uint64_t playerLocation;
        uint64_t unitData;
    };

    ModConfig* setAddresses(Addresses&& addresses);
    const Addresses& getAddresses() const;

private:
    Addresses m_addresses;
};

} // namespace AutomataMod
