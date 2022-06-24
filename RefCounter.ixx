module;

#include <Windows.h>
#include <mutex>
#include <functional>

export module RefCounter;

namespace DxWrappers {

export class RefCounter {
	std::mutex m_mutex;
	ULONG m_refCount;

public:
	RefCounter() {
		m_refCount = 1;
	}

	ULONG getRefCount() const {
		return m_refCount;
	}

	ULONG incrementRef() {
		m_mutex.lock();
		++m_refCount;
		m_mutex.unlock();
		return m_refCount;
	}

	ULONG decrementRef() {
		m_mutex.lock();
		if (m_refCount > 0) {
			--m_refCount;
		}

		m_mutex.unlock();
		return m_refCount;
	}

	ULONG decrementRef(std::function<void(ULONG)> block) {
		m_mutex.lock();
		if (m_refCount > 0) {
			--m_refCount;
		}

		block(m_refCount);

		m_mutex.unlock();
		return m_refCount;
	}
};

} // namespace DxWrappers
