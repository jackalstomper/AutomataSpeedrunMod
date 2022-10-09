#pragma once

#include <utility>

namespace AutomataMod {

template <typename T> class PointerIterator {
	T *_i;

public:
	PointerIterator() : _i(nullptr) {}
	PointerIterator(const PointerIterator &other) : _i(other._i) {}
	PointerIterator(PointerIterator &&other) : PointerIterator() { swap(*this, other); }
	PointerIterator(T *p) : _i(p) {}

	bool operator==(const PointerIterator &other) { return _i == other._i; }

	bool operator!=(const PointerIterator &other) { return !(*this == other); }

	PointerIterator &operator++() {
		++_i;
		return *this;
	}

	PointerIterator operator++(int) {
		T tmp = _i;
		++tmp;
		return tmp;
	}

	T *operator->() { return _i; }
	T &operator*() { return *_i; }
	operator bool() { return _i != nullptr; }

	PointerIterator &operator=(PointerIterator other) {
		swap(*this, other);
		return *this;
	}

	friend void swap(PointerIterator &l, PointerIterator &r) { std::swap(l._i, r._i); }
};

} // namespace AutomataMod
