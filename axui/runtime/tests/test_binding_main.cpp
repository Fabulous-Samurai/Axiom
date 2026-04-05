#include <iostream>
#include <functional>
#include <string>
#include <QCoreApplication>

// Forward declarations
void test_binding_single_update();
void test_binding_multiple_children();
void test_binding_shared_path();
void test_binding_batch_update();
void test_binding_string_value();
void test_binding_bool_value();
void test_binding_color_value();
void test_binding_int64_promotion();
void test_binding_late_binding();
void test_binding_theme_tokens_untouched();
void test_binding_statistics();
void test_binding_source_query();
void test_binding_clear_and_rebind();
void test_binding_rapid_updates();
void test_binding_thread_safety();
void test_binding_singleton();

int main(int argc, char* argv[]) {
    std::cout << "[INFO] AXUI Binding Test Process Starting..." << std::endl;
    // [MANDATORY]: Qt based tests require a QCoreApplication to handle signals/slots
    QCoreApplication app(argc, argv);

    std::cout << "\n═══ AXUI Phase 3: Data Binding Tests ═══\n\n";

    int passed = 0;
    int failed = 0;

    auto run = [&](const char* name, std::function<void()> fn) {
        try {
            fn();
            std::cout << "  ✅ " << name << "\n";
            passed++;
        } catch (const std::exception& e) {
            std::cout << "  ❌ " << name << " — " << e.what() << "\n";
            failed++;
        } catch (...) {
            std::cout << "  ❌ " << name << " — unknown error\n";
            failed++;
        }
    };

    run("binding_single_update",         test_binding_single_update);
    run("binding_multiple_children",     test_binding_multiple_children);
    run("binding_shared_path",           test_binding_shared_path);
    run("binding_batch_update",          test_binding_batch_update);
    run("binding_string_value",          test_binding_string_value);
    run("binding_bool_value",            test_binding_bool_value);
    run("binding_color_value",           test_binding_color_value);
    run("binding_int64_promotion",       test_binding_int64_promotion);
    run("binding_late_binding",          test_binding_late_binding);
    run("binding_theme_tokens_untouched",test_binding_theme_tokens_untouched);
    run("binding_statistics",            test_binding_statistics);
    run("binding_source_query",          test_binding_source_query);
    run("binding_clear_and_rebind",      test_binding_clear_and_rebind);
    run("binding_rapid_updates",         test_binding_rapid_updates);
    run("binding_thread_safety",         test_binding_thread_safety);
    run("binding_singleton",             test_binding_singleton);

    std::cout << "\n═══ Results: " << passed << " passed, "
              << failed << " failed ═══\n\n";

    return failed > 0 ? 1 : 0;
}
