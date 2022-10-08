#include "ModConfig.hpp"

#include <utility>

namespace AutomataMod {

ModConfig *ModConfig::setAddresses(Addresses &&addresses) {
	m_addresses = std::move(addresses);
	return this;
}

const Addresses &ModConfig::getAddresses() const { return m_addresses; }

} // namespace AutomataMod
