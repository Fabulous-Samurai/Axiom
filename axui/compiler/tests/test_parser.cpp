#include "axui/parser.h"
#include "axui/compiler.h"
#include <cassert>
#include <cmath>
#include <iostream>

void test_parser_minimal() {
    axui::Parser parser;
    auto root = parser.parse(R"({
        "$schema": "axui/1.0",
        "root": {
            "component": "Column",
            "layout": { "fill": true, "padding": 24 }
        }
    })");

    assert(!parser.hasErrors());
    assert(root.component_type == axui::ComponentType::Column);
    assert(root.layout.fill_width == true);
    assert(root.layout.fill_height == true);
    assert(std::abs(root.layout.padding[0] - 24.0f) < 0.01f);
}

void test_parser_glass() {
    axui::Parser parser;
    auto root = parser.parse(R"({
        "root": {
            "component": "GlassPanel",
            "glass": {
                "blur": 64,
                "opacity": 0.15,
                "borderOpacity": 0.25,
                "noiseStrength": 0.02
            }
        }
    })");

    assert(!parser.hasErrors());
    assert(root.glass.enabled == true);
    assert(std::abs(root.glass.blur_radius - 64.0f) < 0.01f);
    assert(std::abs(root.glass.bg_opacity - 0.15f) < 0.01f);
    assert(std::abs(root.glass.border_opacity - 0.25f) < 0.01f);
}

void test_parser_hover() {
    axui::Parser parser;
    auto root = parser.parse(R"({
        "root": {
            "component": "KPICard",
            "hover": {
                "scale": 1.05,
                "glowRadius": 16,
                "glowColor": "#7DD3FC",
                "glowOpacity": 0.4,
                "transition": { "duration": 300, "easing": "outCubic" }
            }
        }
    })");

    assert(!parser.hasErrors());
    assert(root.hover.enabled == true);
    assert(std::abs(root.hover.scale - 1.05f) < 0.01f);
    assert(std::abs(root.hover.glow_radius - 16.0f) < 0.01f);
    assert(root.hover.glow_color.r == 125);
    assert(root.hover.glow_color.g == 211);
    assert(root.hover.transition_ms == 300);
    assert(root.hover.easing == axui::Easing::OutCubic);
}

void test_parser_children() {
    axui::Parser parser;
    auto root = parser.parse(R"({
        "root": {
            "component": "Column",
            "children": [
                { "component": "Text", "props": { "text": "Hello" } },
                { "component": "Button", "props": { "text": "Click" } },
                { "component": "Spacer" }
            ]
        }
    })");

    assert(!parser.hasErrors());
    assert(root.children.size() == 3);
    assert(root.children[0].component_type == axui::ComponentType::Text);
    assert(root.children[1].component_type == axui::ComponentType::Button);
    assert(root.children[2].component_type == axui::ComponentType::Spacer);
}

void test_parser_binding() {
    axui::Parser parser;
    auto root = parser.parse(R"({
        "root": {
            "component": "KPICard",
            "props": {
                "title": "Throughput",
                "bind": "@engine.throughput",
                "color": "#10B981"
            }
        }
    })");

    if (parser.hasErrors()) {
        for (auto& e : parser.errors()) std::cout << "Error: " << e.message << "\n";
        throw std::runtime_error("Parser errors");
    }

    auto* bind_prop = root.getProperty("bind");
    if (!bind_prop) throw std::runtime_error("bind prop not found");
    
    if (!std::holds_alternative<axui::Binding>(*bind_prop)) {
        throw std::runtime_error("bind prop is not a Binding");
    }
    
    auto& binding = std::get<axui::Binding>(*bind_prop);
    if (binding.path != "@engine.throughput") throw std::runtime_error("Wrong binding path");

    auto* color_prop = root.getProperty("color");
    if (!color_prop) throw std::runtime_error("color prop not found");
    if (!std::holds_alternative<axui::Color>(*color_prop)) throw std::runtime_error("color prop is not a Color");
}

void test_parser_empty() {
    axui::Parser parser;
    auto root = parser.parse(R"({
        "root": { "component": "Container" }
    })");

    assert(!parser.hasErrors());
    assert(root.component_type == axui::ComponentType::Container);
}

void test_parser_invalid_json() {
    axui::Parser parser;
    auto root = parser.parse("{ invalid json }}}");
    assert(parser.hasErrors());
}

void test_compiler_basic() {
    axui::Compiler compiler;
    auto result = compiler.compile(R"({
        "root": {
            "component": "Column",
            "children": [
                { "component": "Text" }
            ]
        }
    })");
    assert(result.success);
}
