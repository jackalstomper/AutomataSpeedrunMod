#include "RefCounter.hpp"

namespace DxWrappers {

RefCounter::RefCounter() {
	m_refCount = 1;
}

ULONG RefCounter::getRefCount() const {
	return m_refCount;
}

ULONG RefCounter::incrementRef() {
	m_mutex.lock();
	++m_refCount;
	m_mutex.unlock();
	return m_refCount;
}

ULONG RefCounter::decrementRef() {
	m_mutex.lock();
	if (m_refCount > 0) {
		--m_refCount;
	}

	m_mutex.unlock();
	return m_refCount;
}

ULONG RefCounter::decrementRef(std::function<void(ULONG)> block) {
	m_mutex.lock();
	if (m_refCount > 0) {
		--m_refCount;
	}

	block(m_refCount);

	m_mutex.unlock();
	return m_refCount;
}

} // namespace DxWrappers