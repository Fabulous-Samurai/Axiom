#include <gtest/gtest.h>

#include <iostream>

class HarmonicString {
 public:
  HarmonicString(size_t capacity) : capacity_(capacity) {}

  bool assign_from_view(const std::string& str) {
    if (str.size() > capacity_) {
      return false;
    }
    value_ = str;
    return true;
  }

 private:
  size_t capacity_;
  std::string value_;
};

TEST(HarmonicStringTest, SmallStringOptimization) {
  HarmonicString str(16);
  EXPECT_TRUE(str.assign_from_view("short string"));
  EXPECT_FALSE(str.assign_from_view("this string is way too long"));
}

int main() {
  std::cout << "Running Harmonic String Tests..." << std::endl;
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
