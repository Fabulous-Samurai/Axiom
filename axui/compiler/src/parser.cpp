#include "axui/parser.h"
#include <simdjson.h>
#include <iostream>

namespace axui {

UINode Parser::parse(const std::string& json_content) {
    errors_.clear();

    try {
        simdjson::ondemand::parser sjparser;
        simdjson::padded_string padded_json(json_content);
        auto doc = sjparser.iterate(padded_json);

        UINode root;
        auto root_obj = doc.get_object().value();

        // Check if it's a wrapped "root" or direct node
        auto root_field = root_obj.find_field("root");
        if (!root_field.error()) {
            auto obj = root_field.get_object().value();
            root = parseNode(obj);
        } else {
            // Reset and re-parse as direct node (actually iterate again)
            auto doc2 = sjparser.iterate(padded_json);
            auto obj2 = doc2.get_object().value();
            root = parseNode(obj2);
        }
        return root;

    } catch (simdjson::simdjson_error& e) {
        errors_.push_back({std::string("JSON parse error: ") + e.what(), 0});
    } catch (std::exception& e) {
        errors_.push_back({std::string("Error: ") + e.what(), 0});
    }

    return UINode{};
}

UINode Parser::parseNode(simdjson::ondemand::object& obj) {
    UINode node;

    for (auto field : obj) {
        auto key = field.unescaped_key().value();
        
        if (key == "component") {
            auto name = field.value().get_string().value();
            node.component_type = componentTypeFromString(name);
            if (node.id.empty()) node.id = std::string(name);
        } else if (key == "id") {
            node.id = std::string(field.value().get_string().value());
        } else if (key == "glass") {
            auto val = field.value();
            if (val.type().value() == simdjson::ondemand::json_type::object) {
                auto g_obj = val.get_object().value();
                node.glass = parseGlass(g_obj);
            } else {
                node.glass.enabled = val.get_bool().value();
                if (node.glass.enabled) {
                    node.glass.blur_radius = 32.0f;
                    node.glass.bg_opacity = 0.1f;
                    node.glass.border_opacity = 0.2f;
                }
            }
        } else if (key == "hover") {
            auto val = field.value();
            if (val.type().value() == simdjson::ondemand::json_type::object) {
                auto h_obj = val.get_object().value();
                node.hover = parseHover(h_obj);
            } else {
                node.hover.enabled = val.get_bool().value();
                if (node.hover.enabled) {
                    node.hover.scale = 1.02f;
                    node.hover.glow_radius = 12.0f;
                    node.hover.glow_opacity = 0.3f;
                }
            }
        } else if (key == "layout") {
            auto l_obj = field.value().get_object().value();
            node.layout = parseLayout(l_obj);
        } else if (key == "props") {
            auto p_obj = field.value().get_object().value();
            node.properties = parseProperties(p_obj);
        } else if (key == "background") {
            node.properties.push_back({"background", std::string(field.value().get_string().value())});
        } else if (key == "children") {
            auto arr = field.value().get_array().value();
            for (auto child_val : arr) {
                auto c_obj = child_val.get_object().value();
                node.children.push_back(parseNode(c_obj));
            }
        }
    }

    return node;
}

GlassParams Parser::parseGlass(simdjson::ondemand::object& obj) {
    GlassParams g;
    g.enabled = true;
    for (auto field : obj) {
        auto key = field.unescaped_key().value();
        if (key == "blur") g.blur_radius = static_cast<float>(field.value().get_double().value());
        else if (key == "opacity") g.bg_opacity = static_cast<float>(field.value().get_double().value());
        else if (key == "borderOpacity") g.border_opacity = static_cast<float>(field.value().get_double().value());
        else if (key == "noiseStrength") g.noise_strength = static_cast<float>(field.value().get_double().value());
        else if (key == "saturation") g.saturation = static_cast<float>(field.value().get_double().value());
    }
    return g;
}

HoverParams Parser::parseHover(simdjson::ondemand::object& obj) {
    HoverParams h;
    h.enabled = true;
    for (auto field : obj) {
        auto key = field.unescaped_key().value();
        if (key == "scale") h.scale = static_cast<float>(field.value().get_double().value());
        else if (key == "glowRadius") h.glow_radius = static_cast<float>(field.value().get_double().value());
        else if (key == "glowColor") h.glow_color = Color::fromHex(field.value().get_string().value());
        else if (key == "glowOpacity") h.glow_opacity = static_cast<float>(field.value().get_double().value());
        else if (key == "shadowOffset") h.shadow_offset = static_cast<float>(field.value().get_double().value());
        else if (key == "transition") {
            auto t_obj = field.value().get_object().value();
            for (auto t_field : t_obj) {
                auto t_key = t_field.unescaped_key().value();
                if (t_key == "duration") h.transition_ms = static_cast<uint16_t>(t_field.value().get_int64().value());
                else if (t_key == "easing") h.easing = easingFromString(t_field.value().get_string().value());
            }
        }
    }
    return h;
}

LayoutParams Parser::parseLayout(simdjson::ondemand::object& obj) {
    LayoutParams l;
    for (auto field : obj) {
        auto key = field.unescaped_key().value();
        if (key == "type") {
            auto t = field.value().get_string().value();
            if (t == "row") l.type = LayoutType::Row;
            else if (t == "column") l.type = LayoutType::Column;
            else if (t == "grid") l.type = LayoutType::Grid;
            else if (t == "stack") l.type = LayoutType::Stack;
        } else if (key == "width") l.width = static_cast<float>(field.value().get_double().value());
        else if (key == "height") l.height = static_cast<float>(field.value().get_double().value());
        else if (key == "gap") l.gap = static_cast<float>(field.value().get_double().value());
        else if (key == "padding") {
            auto val = field.value();
            if (val.type().value() == simdjson::ondemand::json_type::number) {
                float p = static_cast<float>(val.get_double().value());
                l.padding[0] = l.padding[1] = l.padding[2] = l.padding[3] = p;
            }
        } else if (key == "columns") l.columns = static_cast<uint8_t>(field.value().get_int64().value());
        else if (key == "flex") l.flex = static_cast<uint8_t>(field.value().get_int64().value());
        else if (key == "fill") {
            if (field.value().get_bool().value()) { l.fill_width = true; l.fill_height = true; }
        }
    }
    return l;
}

std::vector<Property> Parser::parseProperties(simdjson::ondemand::object& obj) {
    std::vector<Property> props;
    for (auto field : obj) {
        Property p;
        p.key = std::string(field.unescaped_key().value());
        auto val = field.value();
        auto type = val.type().value();

        if (type == simdjson::ondemand::json_type::string) {
            auto str = std::string(val.get_string().value());
            std::string_view sv = str;
            if (!sv.empty() && sv[0] == ' ') sv.remove_prefix(1);
            
            if (sv.starts_with("@")) p.value = parseBinding(str);
            else if (sv.starts_with("#")) p.value = Color::fromHex(sv);
            else p.value = str;
        } else if (type == simdjson::ondemand::json_type::number) {
            p.value = val.get_double().value();
        } else if (type == simdjson::ondemand::json_type::boolean) {
            p.value = val.get_bool().value();
        } else {
            p.value = std::string("[complex]");
        }
        props.push_back(std::move(p));
    }
    return props;
}

Binding Parser::parseBinding(std::string_view expr) {
    Binding b;
    b.path = std::string(expr);
    b.is_bound = true;
    auto colon = expr.find(':');
    if (colon != std::string_view::npos) {
        b.path = std::string(expr.substr(0, colon));
        b.format = std::string(expr.substr(colon + 1));
    }
    return b;
}

} // namespace axui
