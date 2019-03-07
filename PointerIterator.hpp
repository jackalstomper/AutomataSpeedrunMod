#pragma once

namespace AutomataMod {

template<typename T>
class PointerIterator
{
    T* _i;

public:
    PointerIterator() : _i(nullptr) {}
    PointerIterator(const PointerIterator& other) : _i(other._i) {}
    PointerIterator(T* p) : _i(p) {}

    bool operator==(const PointerIterator& other)
    {
        return _i == other._i;
    }

    bool operator!=(const PointerIterator& other)
    {
        return !(*this == other);
    }

    PointerIterator& operator++()
    {
        ++_i;
        return *this;
    }

    PointerIterator operator++(int)
    {
        T tmp = *this;
        ++tmp;
        return tmp;
    }

    T* operator->()
    {
        return _i;
    }

    T& operator*()
    {
        return *_i;
    }

    bool operator()()
    {
        return _i != nullptr;
    }
};

} // namespace AutomataMod