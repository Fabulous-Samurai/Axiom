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

    // Check if we need normalization
    if (sv.front() != '.' && sv.back() != '.') {
        double result;
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
        if (ec == std::errc{} && ptr == sv.data() + sv.size()) {
            return result;
        }
    }

    // Slow path: Handle ".5", "5." and un-null-terminated string edge cases using stack buffer.
    char buf[64];
    size_t len = sv.size();
    if (len > 60) return std::nullopt; // Too long for reasonable double representation

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

    double result;
    auto [ptr2, ec2] = std::from_chars(buf, buf + i, result);
    return (ec2 == std::errc{} && ptr2 == buf + i) ? std::optional<double>(result) : std::nullopt;
}

int main() {
    std::vector<std::string> inputs = {"1.5", "-2.3456", "100.0", "5.1e3", "0.001", "3.14159"};

    auto start1 = std::chrono::high_resolution_clock::now();
    double sum1 = 0;
    for (int i=0; i<5000000; ++i) {
        for (const auto& s : inputs) {
            sum1 += FastParseDoubleOld(s).value_or(0.0);
        }
    }
    auto end1 = std::chrono::high_resolution_clock::now();

    auto start2 = std::chrono::high_resolution_clock::now();
    double sum2 = 0;
    for (int i=0; i<5000000; ++i) {
        for (const auto& s : inputs) {
            sum2 += FastParseDoubleNew(s).value_or(0.0);
        }
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    std::cout << "Old: " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << "ms, sum: " << sum1 << std::endl;
    std::cout << "New: " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << "ms, sum: " << sum2 << std::endl;
    return 0;
}
