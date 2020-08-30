#pragma once

#include <utility>
#include <iterator>

namespace AutomataMod {

template<typename T>
class PointerIterator : public std::iterator<std::output_iterator_tag, T> {
    T* _i;

public:
    PointerIterator() : _i(nullptr) {}
    PointerIterator(const PointerIterator& other) : _i(other._i) {}
    PointerIterator(T* p) : _i(p) {}
    PointerIterator(PointerIterator&& other) noexcept : PointerIterator() {
        swap(*this, other);
    }

    void swap(PointerIterator& first, PointerIterator& second) noexcept {
        using std::swap;
        swap(first._i, second._i);
    }

    PointerIterator& operator=(PointerIterator other) {
        swap(*this, other);
        return *this;
    }

    bool operator==(const PointerIterator& other) {
        return _i == other._i;
    }

    bool operator!=(const PointerIterator& other) {
        return !(*this == other);
    }

    PointerIterator& operator++() {
        ++_i;
        return *this;
    }

    PointerIterator operator++(int) {
        T tmp = *this;
        ++tmp;
        return tmp;
    }

    T* operator->() {
        return _i;
    }

    T& operator*() {
        return *_i;
    }

    bool operator()() {
        return _i != nullptr;
    }
};

} // namespace AutomataMod