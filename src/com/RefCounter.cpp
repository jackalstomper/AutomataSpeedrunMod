#include "RefCounter.hpp"

namespace DxWrappers {

RefCounter::RefCounter() { m_refCount = 1; }

ULONG RefCounter::getRefCount() const { return m_refCount; }

ULONG RefCounter::incrementRef() {
	std::lock_guard<std::mutex> lock(m_mutex);
	++m_refCount;
	return m_refCount;
}

ULONG RefCounter::decrementRef() {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_refCount > 0) {
		--m_refCount;
	}

	return m_refCount;
}

ULONG RefCounter::decrementRef(std::function<void(ULONG)> block) {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_refCount > 0) {
		--m_refCount;
	}

	block(m_refCount);
	return m_refCount;
}

} // namespace DxWrappers
