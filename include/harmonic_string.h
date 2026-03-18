#pragma once

#include <string>
#include <stdexcept>
#include <memory>
#include <algorithm>

class HarmonicString {
public:
    explicit HarmonicString(std::size_t capacity) noexcept 
        : data_(capacity > 0 ? new(std::nothrow) char[capacity] : nullptr), 
          capacity_(capacity), 
          size_(0) 
    {}

    ~HarmonicString() = default;

    // Rule of 5
    HarmonicString(const HarmonicString& other) : capacity_(other.capacity_), size_(other.size_) {
        if (capacity_ > 0) {
            data_ = std::make_unique<char[]>(capacity_);
            std::copy(other.data_.get(), other.data_.get() + size_, data_.get());
        }
    }

    HarmonicString& operator=(const HarmonicString& other) {
        if (this != &other) {
            capacity_ = other.capacity_;
            size_ = other.size_;
            if (capacity_ > 0) {
                data_ = std::make_unique<char[]>(capacity_);
                std::copy(other.data_.get(), other.data_.get() + size_, data_.get());
            } else {
                data_.reset();
            }
        }
        return *this;
    }

    HarmonicString(HarmonicString&&) noexcept = default;
    HarmonicString& operator=(HarmonicString&&) noexcept = default;

    [[nodiscard]] bool is_valid() const noexcept {
        return data_ != nullptr || capacity_ == 0;
    }

    bool append(const std::string& str) noexcept {
        if (!data_ || size_ + str.size() > capacity_) [[unlikely]] {
            return false;
        }
        std::copy(str.begin(), str.end(), data_.get() + size_);
        size_ += str.size();
        return true;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] const char* c_str() const noexcept {
        return data_.get();
    }

private:
    std::unique_ptr<char[]> data_;
    std::size_t capacity_;
    std::size_t size_;
};
