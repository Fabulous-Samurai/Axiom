#include "axui/resolver.h"
#include "axui/compiler.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        // Fallback: inline tema
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

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
    assert(resolver.currentTheme().displayName == "Test");
    assert(resolver.colorCount() == 7);
}

void test_resolver_color_lookup() {
    axui::ThemeResolver resolver;
    resolver.loadTheme(MINI_THEME);

    auto primary = resolver.resolveColor(" @colors.primary");
    assert(primary.r == 125);
    assert(primary.g == 211);
    assert(primary.b == 252);
    assert(primary.a == 255);

    auto surface = resolver.resolveColor("@colors.surface");
    assert(surface.r == 30);
    assert(surface.g == 41);
    assert(surface.b == 59);
}

void test_resolver_number_lookup() {
    axui::ThemeResolver resolver;
    resolver.loadTheme(MINI_THEME);

    assert(std::abs(resolver.resolveNumber("@typography.h1") - 32.0) < 0.01);
    assert(std::abs(resolver.resolveNumber("@typography.body") - 13.0) < 0.01);
    assert(std::abs(resolver.resolveNumber("@spacing.medium") - 16.0) < 0.01);
    assert(std::abs(resolver.resolveNumber("@radius.xl") - 16.0) < 0.01);
    assert(std::abs(resolver.resolveNumber("@animation.fast") - 100.0) < 0.01);
    assert(std::abs(resolver.resolveNumber("@glass.defaultBlur") - 48.0) < 0.01);
}

void test_resolver_string_lookup() {
    axui::ThemeResolver resolver;
    resolver.loadTheme(MINI_THEME);

    assert(resolver.resolveString("@typography.headingFamily") == "JetBrains Mono");
    assert(resolver.resolveString(" @typography.bodyFamily") == "Inter");
}

void test_resolver_token_parsing() {
    auto parts1 = axui::ThemeResolver::parseToken("@colors.primary");
    assert(parts1.category == "colors");
    assert(parts1.key == "primary");

    auto parts2 = axui::ThemeResolver::parseToken("@typography.h1");
    assert(parts2.category == "typography");
    assert(parts2.key == "h1");

    auto parts3 = axui::ThemeResolver::parseToken("@spacing.medium");
    assert(parts3.category == "spacing");
    assert(parts3.key == "medium");

    auto parts4 = axui::ThemeResolver::parseToken("not_a_token");
    assert(parts4.category == "");
}

void test_resolver_resolve_tree() {
    axui::Compiler compiler;
    auto result = compiler.compile(R"({
        "root": {
            "component": "Column",
            "props": {
                "background": " @colors.surface",
                "color": "@colors.textPrimary",
                "fontSize": "@typography.h2",
                "padding": "@spacing.large"
            },
            "children": [
                {
                    "component": "Text",
                    "props": {
                        "text": "Hello",
                        "color": "@colors.primary",
                        "fontFamily": "@typography.headingFamily"
                    }
                }
            ]
        }
    })", MINI_THEME);

    assert(result.success);

    // Root background resolved to Color
    auto* bg = result.root.getProperty("background");
    assert(bg != nullptr);
    auto& bgColor = std::get<axui::Color>(*bg);
    assert(bgColor.r == 30);
    assert(bgColor.g == 41);
    assert(bgColor.b == 59);

    // Root fontSize resolved to number
    auto* fs = result.root.getProperty("fontSize");
    assert(fs != nullptr);
    assert(std::abs(std::get<double>(*fs) - 25.0) < 0.01);

    // Root padding resolved to number
    auto* pad = result.root.getProperty("padding");
    assert(pad != nullptr);
    assert(std::abs(std::get<double>(*pad) - 24.0) < 0.01);

    // Child color resolved
    assert(result.root.children.size() == 1);
    auto* childColor = result.root.children[0].getProperty("color");
    assert(childColor != nullptr);
    auto& cc = std::get<axui::Color>(*childColor);
    assert(cc.r == 125);
    assert(cc.g == 211);

    // Child fontFamily resolved to string
    auto* ff = result.root.children[0].getProperty("fontFamily");
    assert(ff != nullptr);
    assert(std::get<std::string>(*ff) == "JetBrains Mono");
}

void test_resolver_binding_preserved() {
    axui::Compiler compiler;
    auto result = compiler.compile(R"({
        "root": {
            "component": "KPICard",
            "props": {
                "title": "Throughput",
                "bind": " @engine.throughput",
                "color": "@colors.success"
            }
        }
    })", MINI_THEME);

    assert(result.success);

    // @engine.throughput is a BINDING, not a theme token
    // It should remain as string/Binding if it matches binding pattern
    // In our parser, " @..." is parsed as Binding if it's not a theme token?
    // Actually parser.cpp parses " @..." as Binding.
    
    auto* bind = result.root.getProperty("bind");
    assert(bind != nullptr);
    auto& binding = std::get<axui::Binding>(*bind);
    assert(binding.is_bound == true);
    assert(binding.path == " @engine.throughput");

    // @colors.success IS a theme token, should be resolved
    auto* color = result.root.getProperty("color");
    assert(color != nullptr);
    auto& c = std::get<axui::Color>(*color);
    assert(c.r == 110);
    assert(c.g == 231);
    assert(c.b == 183);
}

void test_resolver_glass_defaults() {
    axui::Compiler compiler;
    auto result = compiler.compile(R"({
        "root": {
            "component": "GlassPanel",
            "glass": true
        }
    })", MINI_THEME);

    assert(result.success);
    assert(result.root.glass.enabled == true);
    // Glass defaults from theme (48, 0.15, 0.25, 0.02)
    assert(std::abs(result.root.glass.blur_radius - 48.0f) < 0.01f);
    assert(std::abs(result.root.glass.bg_opacity - 0.15f) < 0.01f);
    assert(std::abs(result.root.glass.border_opacity - 0.25f) < 0.01f);
    assert(std::abs(result.root.glass.noise_strength - 0.02f) < 0.01f);
}

void test_resolver_theme_swap() {
    axui::ThemeResolver resolver;
    resolver.loadTheme(MINI_THEME);

    auto primary1 = resolver.resolveColor(" @colors.primary");
    assert(primary1.r == 125); // #7DD3FC

    // Swap to different theme
    const char* dark_theme = R"({
        "name": "dark",
        "colors": { "primary": "#38BDF8" },
        "typography": {}, "spacing": {}, "radius": {},
        "glass": {}, "animation": {}
    })";

    axui::UINode dummy;
    resolver.swapTheme(dummy, dark_theme);

    auto primary2 = resolver.resolveColor(" @colors.primary");
    assert(primary2.r == 56); // #38BDF8
    assert(primary2.g == 189);
}

void test_resolver_unknown_token() {
    axui::ThemeResolver resolver;
    resolver.loadTheme(MINI_THEME);

    auto unknown = resolver.resolveColor("@colors.nonexistent");
    assert(unknown.r == 0);
    assert(unknown.g == 0);
    assert(unknown.b == 0);

    auto unknownNum = resolver.resolveNumber("@spacing.nonexistent");
    assert(unknownNum == 0.0);
}

void test_resolver_is_token() {
    axui::ThemeResolver resolver;
    assert(resolver.isToken("@colors.primary") == true);
    assert(resolver.isToken("@typography.h1") == true);
    assert(resolver.isToken("#FF0000") == false);
    assert(resolver.isToken("hello") == false);
    assert(resolver.isToken("") == false);
}
