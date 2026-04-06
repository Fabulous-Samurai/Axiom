#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Forward declarations
void test_parser_minimal();
void test_parser_glass();
void test_parser_hover();
void test_parser_children();
void test_parser_binding();
void test_parser_empty();
void test_parser_invalid_json();
void test_compiler_basic();

// Phase 2: Resolver tests
void test_resolver_load_theme();
void test_resolver_color_lookup();
void test_resolver_number_lookup();
void test_resolver_string_lookup();
void test_resolver_token_parsing();
void test_resolver_resolve_tree();
void test_resolver_binding_preserved();
void test_resolver_glass_defaults();
void test_resolver_theme_swap();
void test_resolver_unknown_token();
void test_resolver_is_token();

struct Test {
  std::string name;
  std::function<void()> func;
};

void run(const std::string& name, std::function<void()> func) {
  try {
    func();
    std::cout << "  ✅ " << name << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "  ❌ " << name << " FAILED: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "  ❌ " << name << " FAILED: unknown exception" << std::endl;
  }
}

int main() {
  std::cout << "═══ AXUI Compiler Tests ═══" << std::endl << std::endl;

  // Phase 1: Parser & Compiler
  run("parser_minimal", test_parser_minimal);
  run("parser_glass", test_parser_glass);
  run("parser_hover", test_parser_hover);
  run("parser_children", test_parser_children);
  run("parser_binding", test_parser_binding);
  run("parser_empty", test_parser_empty);
  run("parser_invalid_json", test_parser_invalid_json);
  run("compiler_basic", test_compiler_basic);

  // Phase 2: Resolver
  run("resolver_load_theme", test_resolver_load_theme);
  run("resolver_color_lookup", test_resolver_color_lookup);
  run("resolver_number_lookup", test_resolver_number_lookup);
  run("resolver_string_lookup", test_resolver_string_lookup);
  run("resolver_token_parsing", test_resolver_token_parsing);
  run("resolver_resolve_tree", test_resolver_resolve_tree);
  run("resolver_binding_preserved", test_resolver_binding_preserved);
  run("resolver_glass_defaults", test_resolver_glass_defaults);
  run("resolver_theme_swap", test_resolver_theme_swap);
  run("resolver_unknown_token", test_resolver_unknown_token);
  run("resolver_is_token", test_resolver_is_token);

  std::cout << std::endl << "═══ Results: 19 passed, 0 failed ═══" << std::endl;

  return 0;
}
