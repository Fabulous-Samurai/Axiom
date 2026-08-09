#include <iostream>
#include <string_view>
#include <charconv>
#include <optional>
#include <string>
#include <chrono>
#include <vector>

std::optional<double> FastParseDoubleOld(std::string_view sv) {
    if (sv.empty()) return std::nullopt;
    std::string str(sv);
    if (str.front() == '.') {
        str = "0" + str;
    } else if (str.back() == '.') {
        str += "0";
    }
    double result;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    return (ec == std::errc{} && ptr == str.data() + str.size()) ? std::optional<double>(result) : std::nullopt;
}

std::optional<double> FastParseDoubleNew(std::string_view sv) {
    if (sv.empty()) return std::nullopt;
    double result;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
    if (ec == std::errc{} && ptr == sv.data() + sv.size()) {
        return result;
    }
    // Fallback for edge cases like missing from_chars support for .5 (though standard should support it, some libc++ might have quirks)
    // Actually from_chars standard does not mandate supporting ".5"?
    // Wait, cppreference: "The pattern is either... A floating-point value... no leading whitespace".
    // Let's use a stack buffer for the edge cases.
    char buf[64];
    size_t len = sv.size();
    if (len > 60) return std::nullopt;

    size_t i = 0;
    if (sv.front() == '.') {
        buf[i++] = '0';
    }
    for (char c : sv) {
        buf[i++] = c;
    }
    if (sv.back() == '.') {
        buf[i++] = '0';
    }

    auto [ptr2, ec2] = std::from_chars(buf, buf + i, result);
    return (ec2 == std::errc{} && ptr2 == buf + i) ? std::optional<double>(result) : std::nullopt;
}

int main() {
    std::vector<std::string> inputs = {"123.456", "0.5", ".5", "5.", "-123.456", "1e-5", "-.5e2"};

    auto start1 = std::chrono::high_resolution_clock::now();
    double sum1 = 0;
    for (int i=0; i<1000000; ++i) {
        for (const auto& s : inputs) {
            sum1 += FastParseDoubleOld(s).value_or(0.0);
        }
    }
    auto end1 = std::chrono::high_resolution_clock::now();

    auto start2 = std::chrono::high_resolution_clock::now();
    double sum2 = 0;
    for (int i=0; i<1000000; ++i) {
        for (const auto& s : inputs) {
            sum2 += FastParseDoubleNew(s).value_or(0.0);
        }
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    std::cout << "Old: " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << "ms, sum: " << sum1 << std::endl;
    std::cout << "New: " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << "ms, sum: " << sum2 << std::endl;
    return 0;
}
