#include <iostream>
#include <string>

int main() {
    std::string s1 = ".5";
    std::string s2 = "5.";
    std::cout << "stod(.5): " << std::stod(s1) << std::endl;
    std::cout << "stod(5.): " << std::stod(s2) << std::endl;
    return 0;
}
