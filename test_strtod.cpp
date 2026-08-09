#include <iostream>
#include <string_view>
#include <cstdlib>

int main() {
    const char* str = "0.5";
    char* endptr;
    double val = std::strtod(str, &endptr);
    std::cout << "val: " << val << " endptr: " << (endptr - str) << std::endl;
    return 0;
}
