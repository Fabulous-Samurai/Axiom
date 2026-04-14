#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include "axui/compiler.h"
#include "axui/resolver.h"

static const char* MINI_THEME = R"({
    "name": "test_theme",
    "displayName": "Test",
    "colors": {
        "surface": "#1E293B",
        "primary": "#7DD3FC",
        "secondary": "#C4B5FD",
        "textPrimary": "#E2E8F0",
        "textSecondary": "#94A3B8",
        "success": "#6EE7B7",
        "error": "#FCA5A5"
    },
    "typography": {
        "headingFamily": "JetBrains Mono",
        "bodyFamily": "Inter",
        "h1": 32, "h2": 25, "h3": 20,
        "body": 13, "small": 12,
        "weightBold": 700, "weightRegular": 400
    },
    "spacing": {
        "small": 8, "medium": 16, "large": 24
    },
    "radius": {
        "medium": 8, "large": 12, "xl": 16
    },
    "glass": {
        "defaultBlur": 48,
        "defaultOpacity": 0.15,
        "defaultBorderOpacity": 0.25,
        "defaultNoiseStrength": 0.02
    },
    "animation": {
        "fast": 100, "normal": 250, "slow": 400
    }
})";

void test_resolver_load_theme() {
  axui::ThemeResolver resolver;
  resolver.loadTheme(MINI_THEME);
  assert(resolver.isLoaded());
  assert(resolver.currentTheme().name == "test_theme");
}

void test_resolver_color_lookup() {
  axui::ThemeResolver resolver;
  resolver.loadTheme(MINI_THEME);
  auto primary = resolver.resolveColor("@colors.primary");
  assert(primary.r == 125);
}

void test_resolver_number_lookup() {
  axui::ThemeResolver resolver;
  resolver.loadTheme(MINI_THEME);
  assert(std::abs(resolver.resolveNumber("@typography.h1") - 32.0) < 0.01);
}

void test_resolver_string_lookup() {
  axui::ThemeResolver resolver;
  resolver.loadTheme(MINI_THEME);
  assert(resolver.resolveString("@typography.headingFamily") ==
         "JetBrains Mono");
}

void test_resolver_token_parsing() {
  auto parts = axui::ThemeResolver::parseToken("@colors.primary");
  assert(parts.category == "colors");
  assert(parts.key == "primary");
}

void test_resolver_resolve_tree() {
  axui::Compiler compiler;
  auto result = compiler.compile(R"({
        "root": {
            "component": "Column",
            "props": {
                "background": "@colors.surface",
                "fontSize": "@typography.h2"
            }
        }
    })",
                                 MINI_THEME);

  assert(result.success);
  assert(result.root != nullptr);
  auto* bg = result.root->getProperty("background");
  assert(bg != nullptr);
  auto* bgColor = std::get_if<axui::Color>(bg);
  assert(bgColor != nullptr);
  assert(bgColor->r == 30);
}

void test_resolver_theme_swap() {
  axui::ThemeResolver resolver;
  resolver.loadTheme(MINI_THEME);
  axui::UINode dummy;
  resolver.swapTheme(dummy, MINI_THEME);
  assert(resolver.isLoaded());
}

void test_resolver_unknown_token() {
  axui::ThemeResolver resolver;
  resolver.loadTheme(MINI_THEME);
  auto unknown = resolver.resolveColor("@colors.nonexistent");
  assert(unknown.r == 0);
}

void test_resolver_is_token() {
  axui::ThemeResolver resolver;
  assert(resolver.isToken("@colors.primary") == true);
  assert(resolver.isToken("hello") == false);
}

void test_resolver_glass_defaults() {
  axui::Compiler compiler;
  auto result = compiler.compile(R"({
        "root": {
            "component": "GlassPanel",
            "glass": true
        }
    })",
                                 MINI_THEME);
  assert(result.success);
  assert(result.root->glass.enabled == true);
}

void test_resolver_binding_preserved() {
  axui::Compiler compiler;
  auto result = compiler.compile(R"({
        "root": {
            "component": "KPICard",
            "props": {
                "title": "Throughput",
                "bind": " @engine.throughput"
            }
        }
    })",
                                 MINI_THEME);

  assert(result.success);
  assert(result.root != nullptr);

  auto* bind = result.root->getProperty("bind");
  assert(bind != nullptr);
  auto* binding = std::get_if<axui::Binding>(bind);
  assert(binding != nullptr);
  assert(binding->is_bound == true);
}
