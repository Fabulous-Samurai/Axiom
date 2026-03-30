#include "axui/binding_engine.h"
#include "axui/compiler.h"
#include <chrono>
#include <iostream>
#include <vector>

using namespace axui;

void bench_single_update() {
    Compiler compiler;
    BindingEngine& engine = BindingContext::instance();
    engine.clearSubscriptions();

    auto result = compiler.compile(R"({
        "root": {
            "component": "KPICard",
            "props": { "value": "@test.path" }
        }
    })");

    if (!result.success || !result.root) return;
    engine.bindTree(*result.root);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        engine.updateSource("test.path", static_cast<double>(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Single update (10k ops): " << duration << "us (" << (duration/10000.0) << "us/op)" << std::endl;
}

int main() {
    bench_single_update();
    return 0;
}
