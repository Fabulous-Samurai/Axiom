#include "axui/parser.h"
#include "axui/compiler.h"
#include "arena_allocator.h"
#include <cassert>
#include <cmath>
#include <iostream>

void test_parser_minimal() {
    axui::Parser parser;
    AXIOM::MemoryArena arena(1024 * 1024);
    AXIOM::ArenaAllocator<axui::UINode> allocator(&arena);

    auto root = parser.parse(R"({
        "$schema": "axui/1.0",
        "root": {
            "component": "Column",
            "layout": { "width": 100, "height": 100 }
        }
    })", allocator);

    assert(!parser.hasErrors());
    assert(root != nullptr);
    assert(root->component_type == axui::ComponentType::Column);
}

void test_parser_glass() {
    axui::Parser parser;
    AXIOM::MemoryArena arena(1024 * 1024);
    AXIOM::ArenaAllocator<axui::UINode> allocator(&arena);

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
    })", allocator);

    assert(!parser.hasErrors());
    assert(root != nullptr);
    assert(root->glass.enabled == true);
    assert(std::abs(root->glass.blur_radius - 64.0f) < 0.01f);
}

void test_parser_hover() {
    axui::Parser parser;
    AXIOM::MemoryArena arena(1024 * 1024);
    AXIOM::ArenaAllocator<axui::UINode> allocator(&arena);

    auto root = parser.parse(R"({
        "root": {
            "component": "KPICard",
            "hover": {
                "scale": 1.05,
                "glowRadius": 16
            }
        }
    })", allocator);

    assert(!parser.hasErrors());
    assert(root != nullptr);
    assert(root->hover.enabled == true);
    assert(std::abs(root->hover.scale - 1.05f) < 0.01f);
}

void test_parser_children() {
    axui::Parser parser;
    AXIOM::MemoryArena arena(1024 * 1024);
    AXIOM::ArenaAllocator<axui::UINode> allocator(&arena);

    auto root = parser.parse(R"({
        "root": {
            "component": "Column",
            "children": [
                { "component": "Text", "props": { "text": "Hello" } },
                { "component": "Button", "props": { "text": "Click" } }
            ]
        }
    })", allocator);

    assert(!parser.hasErrors());
    assert(root != nullptr);
    assert(root->children.size() == 2);
    assert(root->children[0]->component_type == axui::ComponentType::Text);
    assert(root->children[1]->component_type == axui::ComponentType::Button);
}

void test_parser_binding() {
    axui::Parser parser;
    AXIOM::MemoryArena arena(1024 * 1024);
    AXIOM::ArenaAllocator<axui::UINode> allocator(&arena);

    auto root = parser.parse(R"({
        "root": {
            "component": "KPICard",
            "props": {
                "title": "Throughput",
                "bind": "@engine.throughput"
            }
        }
    })", allocator);

    assert(!parser.hasErrors());
    assert(root != nullptr);

    auto* bind_prop = root->getProperty("bind");
    assert(bind_prop != nullptr);
    
    auto* binding = std::get_if<axui::Binding>(bind_prop);
    assert(binding != nullptr);
    assert(binding->path == "@engine.throughput");
}

void test_parser_empty() {
    axui::Parser parser;
    AXIOM::MemoryArena arena(1024 * 1024);
    AXIOM::ArenaAllocator<axui::UINode> allocator(&arena);

    auto root = parser.parse(R"({
        "root": { "component": "Container" }
    })", allocator);

    assert(!parser.hasErrors());
    assert(root->component_type == axui::ComponentType::Container);
}

void test_parser_invalid_json() {
    axui::Parser parser;
    AXIOM::MemoryArena arena(1024 * 1024);
    AXIOM::ArenaAllocator<axui::UINode> allocator(&arena);

    auto root = parser.parse("{ invalid json }}}", allocator);
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
    assert(result.root != nullptr);
}
