#include <iostream>
#include <string_view>
#include <charconv>

int main() {
    std::string_view sv = "5.";
    double result = 0.0;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
    std::cout << "Result: " << result << " EC: " << (int)ec << std::endl;
    return 0;
}
