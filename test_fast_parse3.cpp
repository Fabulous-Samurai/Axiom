#include <iostream>
#include <string_view>
#include <charconv>

int main() {
    std::string_view sv1 = ".5";
    std::string_view sv2 = "5.";
    double result1 = 0.0, result2 = 0.0;

    auto [ptr1, ec1] = std::from_chars(sv1.data(), sv1.data() + sv1.size(), result1);
    std::cout << "Result1: " << result1 << " EC1: " << (int)ec1 << " Parsed len: " << (ptr1 - sv1.data()) << std::endl;

    auto [ptr2, ec2] = std::from_chars(sv2.data(), sv2.data() + sv2.size(), result2);
    std::cout << "Result2: " << result2 << " EC2: " << (int)ec2 << " Parsed len: " << (ptr2 - sv2.data()) << std::endl;
    return 0;
}
