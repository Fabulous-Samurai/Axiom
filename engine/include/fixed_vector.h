// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

namespace AXIOM {

template <typename T, size_t Capacity = 64>
class FixedVector {
 private:
  T data_[Capacity];
  size_t size_{0};

 public:
  FixedVector() = default;

  explicit FixedVector(size_t count, const T& value = T()) {
    assign(count, value);
  }

  explicit FixedVector(std::initializer_list<T> init) {
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

  template <typename... Args>
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

  void resize(size_t new_size) {
    if (new_size <= Capacity) {
      size_ = new_size;
    }
  }

  void clear() { size_ = 0; }
  void reserve(size_t) { /* No-op for fixed capacity */
  }

  bool empty() const { return size_ == 0; }
  bool full() const noexcept { return size_ >= Capacity; }
  size_t size() const { return size_; }
  size_t capacity() const { return Capacity; }

  auto erase(T* it) {
    if (it < begin() || it >= end()) return end();
    size_t index = it - begin();
    for (size_t i = index; i < size_ - 1; ++i) {
      data_[i] = std::move(data_[i + 1]);
    }
    size_--;
    return begin() + index;
  }

  T* data() { return data_; }
  const T* data() const { return data_; }

  T& operator[](size_t index) { return data_[index]; }
  const T& operator[](size_t index) const { return data_[index]; }

  T& front() { return data_[0]; }
  const T& front() const { return data_[0]; }
  T& back() { return data_[size_ > 0 ? size_ - 1 : 0]; }
  const T& back() const { return data_[size_ > 0 ? size_ - 1 : 0]; }

  auto begin() { return data_; }
  auto end() { return data_ + size_; }
  auto begin() const { return data_; }
  auto end() const { return data_ + size_; }
};

// Aliases for LinearSystemParser
template <typename T>
using ArenaVector = FixedVector<T, 64>;

}  // namespace AXIOM
