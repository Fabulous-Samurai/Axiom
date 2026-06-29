#include "axui/compiler.h"
#include <iostream>
#include <chrono>

int main() {
    std::string minimal = R"({
        "root": {
            "component": "Column",
            "layout": { "fill": true, "padding": 24, "gap": 16 },
            "children": [
                { "component": "Text", "props": { "text": "Hello AXUI" } },
                { "component": "GlassPanel", "glass": { "blur": 32, "opacity": 0.1 },
                  "children": [
                    { "component": "KPICard", "props": { "title": "Throughput", "bind": "@engine.ops" } },
                    { "component": "KPICard", "props": { "title": "Memory", "bind": "@engine.mem" } }
                  ]
                },
                { "component": "Chart", "props": { "type": "line" } },
                { "component": "Table", "glass": true }
            ]
        }
    })";

    axui::Compiler compiler;

    // Warmup
    for (int i = 0; i < 10; i++) {
        compiler.compile(minimal);
    }

    // Benchmark
    int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        auto result = compiler.compile(minimal);
        if (!result.success) {
            std::cerr << "Compile failed!\n";
            return 1;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double avg_us = static_cast<double>(total_us) / iterations;

    auto single = compiler.compile(minimal);

    std::cout << "═══ AXUI Parser Benchmark ═══\n";
    std::cout << "  Iterations:     " << iterations << "\n";
    std::cout << "  Total time:     " << total_us / 1000.0 << " ms\n";
    std::cout << "  Average parse:  " << avg_us << " µs\n";
    std::cout << "  Node count:     " << single.node_count << "\n";
    std::cout << "  Compile time:   " << single.compile_time_ms << " ms\n";
    std::cout << "  Target:         < 500 µs (" << (avg_us < 500 ? "✅ PASS" : "❌ FAIL") << ")\n";
    std::cout << "═════════════════════════════\n";

    return avg_us < 500 ? 0 : 1;
}
