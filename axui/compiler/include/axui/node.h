#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>

namespace axui {

struct Binding {
    std::string path;
    std::string format;
    bool is_bound = false;
};

using PropValue = std::variant<
    double,
    std::string,
    bool,
    Color,
    Binding
>;

struct Property {
    std::string key;
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

struct UINode {
    ComponentType component_type = ComponentType::Container;
    std::string id;

    std::vector<Property> properties;
    GlassParams glass;
    HoverParams hover;
    LayoutParams layout;

    std::vector<UINode> children;

    const PropValue* getProperty(const std::string& key) const {
        for (const auto& p : properties) {
            if (p.key == key) return &p.value;
        }
        return nullptr;
    }

    size_t totalNodeCount() const {
        size_t count = 1;
        for (const auto& child : children) {
            count += child.totalNodeCount();
        }
        return count;
    }
};

} // namespace axui
