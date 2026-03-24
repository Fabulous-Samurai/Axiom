#include "axui/binding_engine.h"
#include "axui/compiler.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cassert>

static const char* THEME = R"({
    "name": "bench", "colors": {}, "typography": {},
    "spacing": {}, "radius": {}, "glass": {}, "animation": {}
})";

// ─── Benchmark 1: Tek update süresi ──────────────────────────────

void bench_single_update() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "W",
            "props": { "v": " @engine.data" }
        }
    })", THEME);

    engine.bindTree(result.root);

    const int N = 100000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        engine.updateSource(" @engine.data", static_cast<double>(i));
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_ns = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    );
    double avg_ns = total_ns / N;

    std::cout << "  Single update:     " << std::fixed << std::setprecision(1)
              << avg_ns << " ns/op  (" << N << " iterations)\n";
}

// ─── Benchmark 2: Batch update süresi ────────────────────────────

void bench_batch_update() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    // 5 binding'li tree
    auto result = compiler.compile(R"({
        "root": {
            "component": "D",
            "children": [
                { "component": "A", "props": { "v": " @engine.a" } },
                { "component": "B", "props": { "v": "@engine.b" } },
                { "component": "C", "props": { "v": "@engine.c" } },
                { "component": "D", "props": { "v": "@engine.d" } },
                { "component": "E", "props": { "v": "@engine.e" } }
            ]
        }
    })", THEME);

    engine.bindTree(result.root);

    const int N = 20000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        double v = static_cast<double>(i);
        std::vector<axui::SourceUpdate> batch = {
            {" @engine.a", v * 1}, {"@engine.b", v * 2},
            {"@engine.c", v * 3}, {"@engine.d", v * 4},
            {"@engine.e", v * 5}
        };
        engine.updateBatch(batch);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_ns = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    );
    double avg_ns = total_ns / (N * 5); // 5 update per batch

    std::cout << "  Batch update (5):  " << std::fixed << std::setprecision(1)
              << avg_ns << " ns/op  (" << N << " batches × 5)\n";
}

// ─── Benchmark 3: Fan-out — aynı path'e çok property bağlı ──────

void bench_fanout() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    // 20 child, hepsi aynı path'e bağlı
    std::string json = R"({"root":{"component":"R","children":[)";
    for (int i = 0; i < 20; i++) {
        if (i > 0) json += ",";
        json += R"({"component":"C","props":{"v":" @engine.shared"}})";
    }
    json += "]}}";

    auto result = compiler.compile(json, THEME);
    engine.bindTree(result.root);

    assert(engine.subscriptionCount() == 20);

    const int N = 50000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        engine.updateSource(" @engine.shared", static_cast<double>(i));
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_ns = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    );
    double avg_ns = total_ns / N;

    std::cout << "  Fan-out (1→20):    " << std::fixed << std::setprecision(1)
              << avg_ns << " ns/op  (" << N << " iterations)\n";
}

// ─── Benchmark 4: bindTree süresi ────────────────────────────────

void bench_bind_tree() {
    axui::Compiler compiler;

    // 100 child'lı tree
    std::string json = R"({"root":{"component":"R","children":[)";
    for (int i = 0; i < 100; i++) {
        if (i > 0) json += ",";
        json += R"({"component":"C","props":{"v":" @engine.m)" + std::to_string(i) + R"("}})";
    }
    json += "]}}";

    auto result = compiler.compile(json, THEME);

    const int N = 1000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        axui::BindingEngine engine;
        engine.bindTree(result.root);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_us = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    );
    double avg_us = total_us / N;

    std::cout << "  bindTree (100):    " << std::fixed << std::setprecision(1)
              << avg_us << " µs/call  (" << N << " iterations)\n";
}

// ─── Main ────────────────────────────────────────────────────────

int main() {
    std::cout << "\n═══ AXUI Phase 3: Binding Benchmark ═══\n\n";

    bench_single_update();
    bench_batch_update();
    bench_fanout();
    bench_bind_tree();

    std::cout << "\n═══ Benchmark Complete ═══\n\n";
    return 0;
}
