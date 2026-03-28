#include "axui/parser.h"
#include "arena_allocator.h"
#include <simdjson.h>
#include <iostream>

namespace axui {

UINode* Parser::parse(const std::string& json_content, AXIOM::ArenaAllocator& arena) {
    errors_.clear();

    try {
        simdjson::ondemand::parser sjparser;
        // Padded string is required for simdjson on-demand
        simdjson::padded_string padded_json(json_content);
        auto doc = sjparser.iterate(padded_json);

        auto root_obj = doc.get_object().value();
        UINode* root = arena.allocate_type<UINode>();
        new (root) UINode(); // In-place construction

        auto root_field = root_obj.find_field("root");
        if (!root_field.error()) {
            auto obj = root_field.get_object().value();
            parseNodeInto(obj, *root, arena);
        } else {
            // Re-iterate if "root" key not found
            auto doc2 = sjparser.iterate(padded_json);
            auto obj2 = doc2.get_object().value();
            parseNodeInto(obj2, *root, arena);
        }
        return root;

    } catch (simdjson::simdjson_error& e) {
        errors_.push_back({std::string("JSON parse error: ") + e.what(), 0});
    } catch (std::exception& e) {
        errors_.push_back({std::string("Error: ") + e.what(), 0});
    }

    return nullptr;
}

void Parser::parseNodeInto(simdjson::ondemand::object& obj, UINode& node, AXIOM::ArenaAllocator& arena) {
    for (auto field : obj) {
        auto key = field.unescaped_key().value();
        
        if (key == "component") {
            node.component_type = componentTypeFromString(field.value().get_string().value());
        } else if (key == "id") {
            node.id = field.value().get_string().value();
        } else if (key == "glass") {
            auto val = field.value();
            if (val.type().value() == simdjson::ondemand::json_type::object) {
                auto g_obj = val.get_object().value();
                node.glass = parseGlass(g_obj);
            } else {
                node.glass.enabled = val.get_bool().value();
            }
        } else if (key == "hover") {
            auto val = field.value();
            if (val.type().value() == simdjson::ondemand::json_type::object) {
                auto h_obj = val.get_object().value();
                node.hover = parseHover(h_obj);
            } else {
                node.hover.enabled = val.get_bool().value();
            }
        } else if (key == "layout") {
            auto l_obj = field.value().get_object().value();
            node.layout = parseLayout(l_obj);
        } else if (key == "props") {
            auto p_obj = field.value().get_object().value();
            parsePropertiesInto(p_obj, node, arena);
        } else if (key == "children") {
            auto arr = field.value().get_array().value();
            for (auto child_val : arr) {
                auto c_obj = child_val.get_object().value();
                UINode* child = arena.allocate_type<UINode>();
                new (child) UINode();
                parseNodeInto(c_obj, *child, arena);
                node.children.push_back(child);
            }
        }
    }
}

GlassParams Parser::parseGlass(simdjson::ondemand::object& obj) {
    GlassParams g;
    g.enabled = true;
    for (auto field : obj) {
        auto key = field.unescaped_key().value();
        if (key == "blur") g.blur_radius = static_cast<float>(field.value().get_double().value());
        else if (key == "opacity") g.bg_opacity = static_cast<float>(field.value().get_double().value());
        else if (key == "borderOpacity") g.border_opacity = static_cast<float>(field.value().get_double().value());
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
        } else if (key == "width") l.width = static_cast<float>(field.value().get_double().value());
        else if (key == "height") l.height = static_cast<float>(field.value().get_double().value());
        else if (key == "gap") l.gap = static_cast<float>(field.value().get_double().value());
    }
    return l;
}

void Parser::parsePropertiesInto(simdjson::ondemand::object& obj, UINode& node, AXIOM::ArenaAllocator& arena) {
    for (auto field : obj) {
        Property p;
        p.key = field.unescaped_key().value();
        auto val = field.value();
        auto type = val.type().value();

        if (type == simdjson::ondemand::json_type::string) {
            auto sv = val.get_string().value();
            if (sv.starts_with("@")) p.value = parseBinding(sv);
            else if (sv.starts_with("#")) p.value = Color::fromHex(sv);
            else p.value = sv;
        } else if (type == simdjson::ondemand::json_type::number) {
            p.value = val.get_double().value();
        } else if (type == simdjson::ondemand::json_type::boolean) {
            p.value = val.get_bool().value();
        }
        node.properties.push_back(p);
    }
}

Binding Parser::parseBinding(std::string_view expr) {
    Binding b;
    b.path = expr;
    b.is_bound = true;
    auto colon = expr.find(':');
    if (colon != std::string_view::npos) {
        b.path = expr.substr(0, colon);
        b.format = expr.substr(colon + 1);
    }
    return b;
}

} // namespace axui
