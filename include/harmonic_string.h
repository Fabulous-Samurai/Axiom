#pragma once

#include <string>
#include <stdexcept>

class HarmonicString {
public:
    explicit HarmonicString(std::size_t capacity) noexcept : capacity_(capacity), size_(0) {
        if (capacity == 0) {
            data_ = nullptr;
            return;
        }
        data_ = new(std::nothrow) char[capacity];
    }

    ~HarmonicString() {
        delete[] data_;
    }

    [[nodiscard]] bool is_valid() const noexcept {
        return data_ != nullptr;
    }

    bool append(const std::string& str) noexcept {
        if (!data_ || size_ + str.size() > capacity_) [[unlikely]] {
            return false;
        }
        std::copy(str.begin(), str.end(), data_ + size_);
        size_ += str.size();
        return true;
    }

    std::size_t size() const {
        return size_;
    }

private:
    char* data_;
    std::size_t capacity_;
    std::size_t size_;
};
