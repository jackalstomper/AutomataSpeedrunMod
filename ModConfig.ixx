module;

#include <cstdint>
#include <utility>

export module ModConfig;

namespace AutomataMod {

export struct Addresses {
    uint64_t currentPhase;
    uint64_t isWorldLoaded;
    uint64_t playerSetName;
    uint64_t isLoading;
    uint64_t itemTableStart;
    uint64_t chipTableStart;
    uint64_t playerLocation;
    uint64_t unitData;
};

export class ModConfig {
public:

    ModConfig* setAddresses(Addresses&& addresses) {
        m_addresses = std::move(addresses);
        return this;
    }

    const Addresses& getAddresses() const {
        return m_addresses;
    }

private:
    Addresses m_addresses;
};

} // namespace AutomataMod
