#pragma once

#include <memory>
#include <wrl/client.h>

// Takes ownership of a wrapper raw pointer and controls its reference count and lifetime
template <typename T> class WrapperPointer {
	std::unique_ptr<T> _obj;
	Microsoft::WRL::ComPtr<T> _comPtr;

public:
	WrapperPointer() {}
	WrapperPointer(const WrapperPointer &other) = delete;
	WrapperPointer(WrapperPointer &&other) : WrapperPointer() { swap(*this, other); }
	WrapperPointer(decltype(nullptr)) {}
	WrapperPointer(T *ptr) {
		_obj = std::unique_ptr<T>(ptr);
		_comPtr = ptr;
	}

	WrapperPointer &operator=(decltype(nullptr)) {
		_comPtr = nullptr; // Need to clear com pointer first so it can call Release()
		_obj = nullptr;
		return *this;
	}

	WrapperPointer &operator=(T *ptr) {
		if (_obj && _obj.get() == ptr)
			return *this;

		auto other = WrapperPointer(ptr);
		swap(other, *this);
		return *this;
	}

	Microsoft::WRL::ComPtr<T> getComPtr() const {
		Microsoft::WRL::ComPtr<T> p = _comPtr;
		return p;
	}

	operator bool() const { return _obj != nullptr; }
	T *operator->() const { return _obj.get(); }
	T *get() const { return _comPtr.Get(); }

	friend void swap(WrapperPointer &l, WrapperPointer &r) {
		std::swap(l._comPtr, r._comPtr);
		std::swap(l._obj, r._obj);
	}
};
