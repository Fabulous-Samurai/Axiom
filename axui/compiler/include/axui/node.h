#pragma once

#include "types.h"
#include "fixed_vector.h" // AXIOM::FixedVector
#include <string_view>
#include <variant>
#include <optional>

namespace axui {

struct Binding {
    std::string_view path;
    std::string_view format;
    bool is_bound = false;
};

using PropValue = std::variant<
    double,
    std::string_view,
    bool,
    Color,
    Binding
>;

struct Property {
    std::string_view key;
    PropValue value;
};

struct GlassParams {
    float blur_radius = 32.0f;
    float bg_opacity = 0.1f;
    float border_opacity = 0.2f;
    float noise_strength = 0.015f;
    float saturation = 1.2f;
    bool enabled = false;
};

struct HoverParams {
    float scale = 1.0f;
    float glow_radius = 0.0f;
    Color glow_color = {125, 211, 252, 255};
    float glow_opacity = 0.0f;
    float shadow_offset = 0.0f;
    uint16_t transition_ms = 200;
    Easing easing = Easing::OutCubic;
    bool enabled = false;
};

struct LayoutParams {
    float x = 0, y = 0;
    float width = 0, height = 0;
    float margin[4] = {0, 0, 0, 0};
    float padding[4] = {0, 0, 0, 0};
    float gap = 0;
    uint8_t flex = 0;
    uint8_t columns = 1;
    bool fill_width = false;
    bool fill_height = false;
    LayoutType type = LayoutType::Column;
};

// Zenith Limit: Max 32 properties and 16 children per node to maintain L1 cache locality
constexpr size_t MAX_PROPS_PER_NODE = 32;
constexpr size_t MAX_CHILDREN_PER_NODE = 16;

struct UINode {
    ComponentType component_type = ComponentType::Container;
    std::string_view id;

    AXIOM::FixedVector<Property, MAX_PROPS_PER_NODE> properties;
    GlassParams glass;
    HoverParams hover;
    LayoutParams layout;

    AXIOM::FixedVector<UINode*, MAX_CHILDREN_PER_NODE> children;

    const PropValue* getProperty(std::string_view key) const {
        for (const auto& p : properties) {
            if (p.key == key) return &p.value;
        }
        return nullptr;
    }

    size_t totalNodeCount() const {
        size_t count = 1;
        for (const auto& child : children) {
            if (child) count += child->totalNodeCount();
        }
        return count;
    }
};

} // namespace axui
