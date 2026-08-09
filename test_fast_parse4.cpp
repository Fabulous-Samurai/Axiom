#include <iostream>
#include <string>
#include <string_view>
#include <charconv>

int main() {
    std::string_view sv = "123.456";
    std::string str(sv);
    std::cout << "Allocated: " << str << std::endl;
    return 0;
}
