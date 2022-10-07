#pragma once

#include <cstdint>

namespace AutomataMod {

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

class ModConfig {
  Addresses m_addresses;

public:
  ModConfig *setAddresses(Addresses &&addresses);
  const Addresses &getAddresses() const;
};

} // namespace AutomataMod