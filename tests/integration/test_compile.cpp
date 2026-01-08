// Quick compile test for string vs string_view in map
#include <unordered_map>
#include <string>
#include <string_view>
#include <functional>
#include <iostream>

int main() {
    // This should compile fine
    std::unordered_map<std::string, std::function<double(double)>> func_map = {
        {"log2", [](double x) { return x * 2; }},
        {"sqrt", [](double x) { return x * x; }}
    };
    
    std::string_view sv = "log2";
    auto it = func_map.find(std::string(sv));
    
    if (it != func_map.end()) {
        std::cout << "Found: log2" << std::endl;
        std::cout << "Result: " << it->second(5.0) << std::endl;
    }
    
    return 0;
}
