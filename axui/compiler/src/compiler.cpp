#include "axui/compiler.h"
#include <fstream>
#include <sstream>
#include <chrono>

namespace axui {

CompileResult Compiler::compile(const std::string& axui_json) {
    auto start = std::chrono::high_resolution_clock::now();
    CompileResult result;

    result.root = parser_.parse(axui_json, allocator_);

    if (parser_.hasErrors() || !result.root) {
        result.success = false;
        for (const auto& err : parser_.errors()) {
            result.errors.push_back(err.message);
        }
        return result;
    }

    result.success = true;
    result.node_count = result.root->totalNodeCount();

    auto end = std::chrono::high_resolution_clock::now();
    result.compile_time_ms = std::chrono::duration_cast<
        std::chrono::microseconds>(end - start).count() / 1000.0;

    return result;
}

CompileResult Compiler::compile(
    const std::string& axui_json,
    const std::string& theme_json
) {
    auto start = std::chrono::high_resolution_clock::now();

    resolver_.loadTheme(theme_json);

    if (!resolver_.isLoaded()) {
        CompileResult result;
        result.success = false;
        result.errors.push_back("Failed to load theme");
        return result;
    }

    auto result = compile(axui_json);

    if (result.success && result.root) {
        resolver_.resolve(*result.root);
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.compile_time_ms = std::chrono::duration_cast<
        std::chrono::microseconds>(end - start).count() / 1000.0;

    return result;
}

} // namespace axui
