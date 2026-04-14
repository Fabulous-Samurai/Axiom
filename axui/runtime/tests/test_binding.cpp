#include "axui/binding_engine.h"
#include "axui/compiler.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

using namespace axui;

static bool is_near(double a, double b, double eps = 0.001) {
    return std::abs(a - b) < eps;
}

void test_binding_singleton() {
    std::cout << "[RUN] test_binding_singleton" << std::endl;
    BindingEngine& e1 = BindingContext::instance();
    BindingEngine& e2 = BindingContext::instance();
    if (&e1 != &e2) { std::cerr << "Singleton failed!" << std::endl; exit(1); }
}

void test_binding_single_update() {
    std::cout << "[RUN] test_binding_single_update" << std::endl;
    Compiler compiler;
    BindingEngine& engine = BindingContext::instance();
    engine.clearSubscriptions();

    auto result = compiler.compile(R"({
        "root": {
            "component": "KPICard",
            "props": {
                "value": "@engine.cpu_usage"
            }
        }
    })");

    if (!result.success || !result.root) { std::cerr << "Compile failed!" << std::endl; exit(1); }
    engine.bindTree(*result.root);

    engine.updateSource("engine.cpu_usage", 45.5);
    auto* prop = result.root->getProperty("value");
    if (!prop) { std::cerr << "Property not found!" << std::endl; exit(1); }
    if (!is_near(std::get<double>(*prop), 45.5)) { std::cerr << "Value mismatch!" << std::endl; exit(1); }
}

void test_binding_multiple_children() {
    std::cout << "[RUN] test_binding_multiple_children" << std::endl;
    Compiler compiler;
    BindingEngine& engine = BindingContext::instance();
    engine.clearSubscriptions();

    auto result = compiler.compile(R"({
        "root": {
            "component": "Column",
            "children": [
                { "component": "Text", "props": { "value": "@sys.cpu" } },
                { "component": "Text", "props": { "value": "@sys.ram" } }
            ]
        }
    })");

    if (!result.success || !result.root) { std::cerr << "Compile failed!" << std::endl; exit(1); }
    engine.bindTree(*result.root);

    engine.updateSource("sys.cpu", 10.0);
    engine.updateSource("sys.ram", 85.0);

    auto* cpu = result.root->children[0]->getProperty("value");
    auto* ram = result.root->children[1]->getProperty("value");

    if (!is_near(std::get<double>(*cpu), 10.0)) { std::cerr << "CPU mismatch!" << std::endl; exit(1); }
    if (!is_near(std::get<double>(*ram), 85.0)) { std::cerr << "RAM mismatch!" << std::endl; exit(1); }
}

void test_binding_thread_safety() {
    std::cout << "[RUN] test_binding_thread_safety" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    std::atomic<bool> running{true};

    std::thread updater([&]() {
        double val = 0.0;
        while(running) {
            engine.updateSource("test.thread", val);
            val += 1.0;
            std::this_thread::yield();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    running = false;
    updater.join();
}

void test_binding_rapid_updates() {
    std::cout << "[RUN] test_binding_rapid_updates" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    for(int i=0; i<1000; ++i) {
        engine.updateSource("rapid.test", static_cast<double>(i));
    }
}

void test_binding_clear_and_rebind() {
    std::cout << "[RUN] test_binding_clear_and_rebind" << std::endl;
    Compiler compiler;
    BindingEngine& engine = BindingContext::instance();
    engine.clearSubscriptions();

    auto result = compiler.compile(R"({"root":{"component":"Text","props":{"v":"@t"}}})");
    engine.bindTree(*result.root);
    if (engine.subscriptionCount() != 1) { std::cerr << "Sub count mismatch!" << std::endl; exit(1); }

    engine.clearSubscriptions();
    if (engine.subscriptionCount() != 0) { std::cerr << "Clear failed!" << std::endl; exit(1); }
}

void test_binding_source_query() {
    std::cout << "[RUN] test_binding_source_query" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    engine.updateSource("query.test", 123.45);
    if (!engine.hasSource("query.test")) { std::cerr << "Source not found!" << std::endl; exit(1); }
    if (std::get<double>(engine.getSource("query.test")) != 123.45) { std::cerr << "Value mismatch!" << std::endl; exit(1); }
}

void test_binding_statistics() {
    std::cout << "[RUN] test_binding_statistics" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    engine.updateSource("stats.test", 1.0);
    auto s = engine.statistics();
    if (s.total_updates == 0) { std::cerr << "Stats failed!" << std::endl; exit(1); }
}

void test_binding_theme_tokens_untouched() {
    std::cout << "[RUN] test_binding_theme_tokens_untouched" << std::endl;
    Compiler compiler;
    BindingEngine& engine = BindingContext::instance();
    engine.clearSubscriptions();

    auto result = compiler.compile(R"({"root":{"component":"Text","props":{"color":"#FF0000"}}})");
    engine.bindTree(*result.root);
    if (engine.subscriptionCount() != 0) { std::cerr << "Theme token bound incorrectly!" << std::endl; exit(1); }
}

void test_binding_late_binding() {
    std::cout << "[RUN] test_binding_late_binding" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    engine.updateSource("late.val", 99.0);

    Compiler compiler;
    auto result = compiler.compile(R"({"root":{"component":"Text","props":{"v":"@late.val"}}})");
    engine.bindTree(*result.root);

    auto* p = result.root->getProperty("v");
    if (std::get<double>(*p) != 99.0) { std::cerr << "Late binding failed!" << std::endl; exit(1); }
}

void test_binding_int64_promotion() {
    std::cout << "[RUN] test_binding_int64_promotion" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    engine.updateSource("int.val", static_cast<int64_t>(42));

    Compiler compiler;
    auto result = compiler.compile(R"({"root":{"component":"Text","props":{"v":"@int.val"}}})");
    engine.bindTree(*result.root);

    auto* p = result.root->getProperty("v");
    if (std::get<double>(*p) != 42.0) { std::cerr << "Promotion failed!" << std::endl; exit(1); }
}

void test_binding_color_value() {
    std::cout << "[RUN] test_binding_color_value" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    Color c{255, 0, 0, 255};
    engine.updateSource("color.val", c);

    Compiler compiler;
    auto result = compiler.compile(R"({ "root": { "component": "Text", "props": { "c": "@color.val" } } })");
    engine.bindTree(*result.root);

    auto* p = result.root->getProperty("c");
    if (std::get<Color>(*p).r != 255) { std::cerr << "Color binding failed!" << std::endl; exit(1); }
}

void test_binding_bool_value() {
    std::cout << "[RUN] test_binding_bool_value" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    engine.updateSource("bool.val", true);

    Compiler compiler;
    auto result = compiler.compile(R"({ "root": { "component": "Text", "props": { "b": "@bool.val" } } })");
    engine.bindTree(*result.root);

    auto* p = result.root->getProperty("b");
    if (std::get<bool>(*p) != true) { std::cerr << "Bool binding failed!" << std::endl; exit(1); }
}

void test_binding_string_value() {
    std::cout << "[RUN] test_binding_string_value" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    engine.updateSource("str.val", std::string("hello"));

    Compiler compiler;
    auto result = compiler.compile(R"({ "root": { "component": "Text", "props": { "s": "@str.val" } } })");
    engine.bindTree(*result.root);

    auto* p = result.root->getProperty("s");
    if (std::get<std::string_view>(*p) != "hello") { std::cerr << "String binding failed!" << std::endl; exit(1); }
}

void test_binding_batch_update() {
    std::cout << "[RUN] test_binding_batch_update" << std::endl;
    BindingEngine& engine = BindingContext::instance();
    std::vector<SourceUpdate> updates = {
        {"batch.1", 1.0},
        {"batch.2", 2.0}
    };
    engine.updateBatch(updates);
    if (std::get<double>(engine.getSource("batch.1")) != 1.0) { std::cerr << "Batch 1 failed!" << std::endl; exit(1); }
}

void test_binding_shared_path() {
    std::cout << "[RUN] test_binding_shared_path" << std::endl;
    Compiler compiler;
    BindingEngine& engine = BindingContext::instance();
    engine.clearSubscriptions();

    auto result = compiler.compile(R"({
        "root": {
            "component": "Row",
            "children": [
                { "component": "Text", "props": { "v": "@shared" } },
                { "component": "Text", "props": { "v": "@shared" } }
            ]
        }
    })");

    engine.bindTree(*result.root);
    engine.updateSource("shared", 5.5);
    if (std::get<double>(*result.root->children[0]->getProperty("v")) != 5.5) { std::cerr << "Shared path failed!" << std::endl; exit(1); }
}
