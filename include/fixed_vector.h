#pragma once
#include <cstddef>
#include <stdexcept>
#include <initializer_list>


namespace AXIOM {

template<typename T, size_t Capacity = 64>
class FixedVector {
private:
    T data_[Capacity];
    size_t size_{0};

public:
    FixedVector() = default;
    
    FixedVector(size_t count, const T& value = T()) {
        assign(count, value);
    }
    
    FixedVector(std::initializer_list<T> init) {
        for (const auto& item : init) {
            push_back(item);
        }
    }

    void push_back(const T& value) {
        if (size_ < Capacity) {
            data_[size_++] = value;
        }
    }
    
    void push_back(T&& value) {
        if (size_ < Capacity) {
            data_[size_++] = std::move(value);
        }
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (size_ < Capacity) {
            data_[size_++] = T(std::forward<Args>(args)...);
        }
    }

    void assign(size_t count, const T& value) {
        size_ = count > Capacity ? Capacity : count;
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = value;
        }
    }

    void clear() { size_ = 0; }
    void reserve(size_t) { /* No-op for fixed capacity */ }
    
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    size_t capacity() const { return Capacity; }

    T* data() { return data_; }
    const T* data() const { return data_; }

    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }
    
    auto begin() { return data_; }
    auto end() { return data_ + size_; }
    auto begin() const { return data_; }
    auto end() const { return data_ + size_; }
};

// Aliases for LinearSystemParser
template<typename T>
using ArenaVector = FixedVector<T, 64>;

} // namespace AXIOM


