#include <iostream>
#include <string_view>
#include <charconv>
#include <string>
#include <optional>

inline std::optional<double> FastParseDouble(std::string_view sv) {
    if (sv.empty()) return std::nullopt;

    // Fast path: try to parse without allocation (Zero-Allocation rule)
    const char* first = sv.data();
    const char* last = sv.data() + sv.size();

    // Skip leading '+' which from_chars doesn't support
    if (first != last && *first == '+') {
        ++first;
    }

    double result;
#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
    // Fallback for compilers with missing floating-point from_chars
    try {
        std::string str(first, last); // Allocation unavoidable here on old compilers
        size_t pos;
        result = std::stod(str, &pos);
        if (pos != str.size()) return std::nullopt;
        return result;
    } catch (...) {
        return std::nullopt;
    }
#else
    auto [ptr, ec] = std::from_chars(first, last, result);
    if (ec == std::errc{} && ptr == last) {
        return result;
    }

    // Fallback if the strict from_chars fails (e.g. some weird formatting)
    try {
        std::string str(sv); // Fallback allocation
        size_t pos;
        result = std::stod(str, &pos);
        if (pos != str.size()) return std::nullopt;
        return result;
    } catch (...) {
        return std::nullopt;
    }
#endif
}

void test(std::string_view sv) {
    auto res = FastParseDouble(sv);
    if (res) {
        std::cout << sv << " -> " << *res << "\n";
    } else {
        std::cout << sv << " -> fail\n";
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
