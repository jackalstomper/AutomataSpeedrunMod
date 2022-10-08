#pragma once

#include <Windows.h>
#include <functional>
#include <mutex>

namespace DxWrappers {

class RefCounter {
	ULONG m_refCount;
	std::mutex m_mutex;

public:
	RefCounter();
	ULONG getRefCount() const;
	ULONG incrementRef();
	ULONG decrementRef();
	ULONG decrementRef(std::function<void(ULONG)> block);
};

} // namespace DxWrappers
