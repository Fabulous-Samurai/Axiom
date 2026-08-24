#include <iostream>
#include <string_view>
#include <charconv>
#include <string>
#include <optional>

void test(std::string_view sv) {
    double result = 0;
    const char* first = sv.data();
    const char* last = sv.data() + sv.size();
    if (first != last && *first == '+') {
        ++first;
    }
    auto [ptr, ec] = std::from_chars(first, last, result);
    if (ec == std::errc{} && ptr == last) {
        std::cout << sv << " -> " << result << " (fast)\n";
    } else {
        std::cout << sv << " -> fail (fast)\n";
    }
}

int main() {
    test("3.14");
    test("+3.14");
    test(".5");
    test("5.");
    test("inf");
    test("NaN");
    test("1e-5");
    return 0;
}
