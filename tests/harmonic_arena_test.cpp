#include <gtest/gtest.h>

#include <iostream>

class HarmonicArena {
 public:
  HarmonicArena(size_t capacity) : capacity_(capacity), used_(0) {}

  void* allocate(size_t size) {
    if (used_ + size > capacity_) {
      return nullptr;
    }
    used_ += size;
    return reinterpret_cast<void*>(used_);  // Dummy pointer
  }

 private:
  size_t capacity_;
  size_t used_;
};

TEST(HarmonicArenaTest, BoundaryViolation) {
  HarmonicArena arena(1024);
  void* ptr1 = arena.allocate(1024);
  EXPECT_NE(ptr1, nullptr);

  void* ptr2 = arena.allocate(1);
  EXPECT_EQ(ptr2, nullptr);
}

int main() {
  std::cout << "Running Harmonic Arena Tests..." << std::endl;
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
