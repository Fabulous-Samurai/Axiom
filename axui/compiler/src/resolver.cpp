#include "axui/resolver.h"
#include <simdjson.h>
#include <iostream>

namespace axui {

ThemeResolver::TokenParts ThemeResolver::parseToken(const std::string& token) {
    TokenParts parts;
    if (token.empty()) return parts;

    size_t start = 0;
    while (start < token.size() && token[start] == ' ') start++;

    if (start >= token.size() || token[start] != '@') return parts;
    start++; // Skip '@'

    auto dotPos = token.find('.', start);
    if (dotPos == std::string::npos) {
        parts.category = token.substr(start);
        return parts;
    }

    parts.category = token.substr(start, dotPos - start);
    parts.key = token.substr(dotPos + 1);
    return parts;
}

void ThemeResolver::loadTheme(const std::string& theme_json) {
    theme_ = ThemeData{};
    loaded_ = false;

    simdjson::ondemand::parser parser;
    try {
        std::string padded_json = theme_json + std::string(simdjson::SIMDJSON_PADDING, ' ');
        auto doc = parser.iterate(padded_json);
        auto root = doc.get_object().value();

        for (auto field : root) {
            auto key = field.unescaped_key().value();
            auto val = field.value();

            if (key == "name") {
                theme_.name = std::string(val.get_string().value());
            } else if (key == "displayName") {
                theme_.displayName = std::string(val.get_string().value());
            } else if (key == "colors") {
                auto colors_obj = val.get_object().value();
                for (auto cfield : colors_obj) {
                    auto ckey = std::string(cfield.unescaped_key().value());
                    auto cval = std::string(cfield.value().get_string().value());
                    theme_.colors[ckey] = Color::fromHex(cval);
                }
            } else if (key == "typography") {
                auto typo_obj = val.get_object().value();
                for (auto tfield : typo_obj) {
                    auto tkey = std::string(tfield.unescaped_key().value());
                    auto tval = tfield.value();
                    auto ttype = tval.type().value();
                    if (ttype == simdjson::ondemand::json_type::number) {
                        theme_.typography[tkey] = tval.get_double().value();
                    } else if (ttype == simdjson::ondemand::json_type::string) {
                        theme_.typographyStrings[tkey] = std::string(tval.get_string().value());
                    }
                }
            } else if (key == "spacing") {
                auto spacing_obj = val.get_object().value();
                for (auto sfield : spacing_obj) {
                    auto skey = std::string(sfield.unescaped_key().value());
                    theme_.spacing[skey] = sfield.value().get_double().value();
                }
            } else if (key == "radius") {
                auto radius_obj = val.get_object().value();
                for (auto rfield : radius_obj) {
                    auto rkey = std::string(rfield.unescaped_key().value());
                    theme_.radius[rkey] = rfield.value().get_double().value();
                }
            } else if (key == "elevation") {
                auto elev_obj = val.get_object().value();
                for (auto efield : elev_obj) {
                    auto ekey = std::string(efield.unescaped_key().value());
                    auto level_obj = efield.value().get_object().value();
                    ThemeData::ElevationLevel level;

                    for (auto lfield : level_obj) {
                        auto lkey = lfield.unescaped_key().value();
                        if (lkey == "offset") level.offset = lfield.value().get_double().value();
                        else if (lkey == "blur") level.blur = lfield.value().get_double().value();
                        else if (lkey == "opacity") level.opacity = lfield.value().get_double().value();
                    }
                    theme_.elevation[ekey] = level;
                }
            } else if (key == "glass") {
                auto glass_obj = val.get_object().value();
                for (auto gfield : glass_obj) {
                    auto gkey = gfield.unescaped_key().value();
                    if (gkey == "defaultBlur") theme_.glass.defaultBlur = gfield.value().get_double().value();
                    else if (gkey == "defaultOpacity") theme_.glass.defaultOpacity = gfield.value().get_double().value();
                    else if (gkey == "defaultBorderOpacity") theme_.glass.defaultBorderOpacity = gfield.value().get_double().value();
                    else if (gkey == "defaultNoiseStrength") theme_.glass.defaultNoiseStrength = gfield.value().get_double().value();
                }
            } else if (key == "animation") {
                auto anim_obj = val.get_object().value();
                for (auto afield : anim_obj) {
                    auto akey = afield.unescaped_key().value();
                    if (akey == "fast") theme_.animation.fast = afield.value().get_double().value();
                    else if (akey == "normal") theme_.animation.normal = afield.value().get_double().value();
                    else if (akey == "slow") theme_.animation.slow = afield.value().get_double().value();
                }
            }
        }
        loaded_ = true;
    } catch (simdjson::simdjson_error& e) {
        std::cerr << "simdjson error in loadTheme: " << e.what() << std::endl;
        loaded_ = false;
    }
}

Color ThemeResolver::resolveColor(const std::string& token) const {
    auto parts = parseToken(token);
    if (parts.category == "colors") {
        auto it = theme_.colors.find(parts.key);
        if (it != theme_.colors.end()) return it->second;
    }
    return Color{};
}

double ThemeResolver::resolveNumber(const std::string& token) const {
    auto parts = parseToken(token);
    if (parts.category == "typography") {
        auto it = theme_.typography.find(parts.key);
        if (it != theme_.typography.end()) return it->second;
    }
    if (parts.category == "spacing") {
        auto it = theme_.spacing.find(parts.key);
        if (it != theme_.spacing.end()) return it->second;
    }
    if (parts.category == "radius") {
        auto it = theme_.radius.find(parts.key);
        if (it != theme_.radius.end()) return it->second;
    }
    if (parts.category == "animation") {
        if (parts.key == "fast") return theme_.animation.fast;
        if (parts.key == "normal") return theme_.animation.normal;
        if (parts.key == "slow") return theme_.animation.slow;
    }
    if (parts.category == "glass") {
        if (parts.key == "defaultBlur") return theme_.glass.defaultBlur;
        if (parts.key == "defaultOpacity") return theme_.glass.defaultOpacity;
        if (parts.key == "defaultBorderOpacity") return theme_.glass.defaultBorderOpacity;
        if (parts.key == "defaultNoiseStrength") return theme_.glass.defaultNoiseStrength;
    }
    return 0.0;
}

std::string ThemeResolver::resolveString(const std::string& token) const {
    auto parts = parseToken(token);
    if (parts.category == "typography") {
        auto it = theme_.typographyStrings.find(parts.key);
        if (it != theme_.typographyStrings.end()) return it->second;
    }
    return "";
}

size_t ThemeResolver::tokenCount() const {
    return theme_.colors.size() +
           theme_.typography.size() +
           theme_.typographyStrings.size() +
           theme_.spacing.size() +
           theme_.radius.size() +
           theme_.elevation.size();
}

void ThemeResolver::resolve(UINode& root) {
    if (!loaded_) return;
    resolve_count_ = 0;
    resolveNode(root);
}

void ThemeResolver::resolveNode(UINode& node) {
    resolveProperties(node.properties);
    resolveGlassDefaults(node.glass);
    resolveHoverColors(node.hover);

    for (auto* child : node.children) {
        if (child) resolveNode(*child);
    }
}

static bool isThemeCategory(const std::string& cat) {
    return cat == "colors" || cat == "typography" || cat == "spacing" ||
           cat == "radius" || cat == "animation" || cat == "glass";
}

void ThemeResolver::resolveProperties(AXIOM::FixedVector<Property, 32>& props) {
    for (auto& prop : props) {
        std::string token_str;
        bool is_token = false;

        if (auto* sv = std::get_if<std::string_view>(&prop.value)) {
            if (isToken(*sv)) {
                token_str = std::string(*sv);
                is_token = true;
            }
        } else if (auto* binding = std::get_if<Binding>(&prop.value)) {
            if (isToken(binding->path)) {
                token_str = std::string(binding->path);
                is_token = true;
            }
        }

        if (is_token) {
            auto parts = parseToken(token_str);
            if (isThemeCategory(parts.category)) {
                if (parts.category == "colors") {
                    prop.value = resolveColor(token_str);
                    resolve_count_++;
                } else if (parts.category == "typography") {
                    auto num_it = theme_.typography.find(parts.key);
                    if (num_it != theme_.typography.end()) {
                        prop.value = num_it->second;
                        resolve_count_++;
                    } else {
                        auto str_it = theme_.typographyStrings.find(parts.key);
                        if (str_it != theme_.typographyStrings.end()) {
                            prop.value = std::string_view(str_it->second); // Note: lifetime must be managed
                            resolve_count_++;
                        }
                    }
                } else if (parts.category == "spacing" ||
                           parts.category == "radius" ||
                           parts.category == "animation" ||
                           parts.category == "glass") {
                    prop.value = resolveNumber(token_str);
                    resolve_count_++;
                }
            }
        }
    }
}

void ThemeResolver::resolveGlassDefaults(GlassParams& glass) {
    if (glass.enabled && loaded_) {
        if (glass.blur_radius == 32.0f) glass.blur_radius = static_cast<float>(theme_.glass.defaultBlur);
        if (glass.bg_opacity == 0.1f) glass.bg_opacity = static_cast<float>(theme_.glass.defaultOpacity);
        if (glass.border_opacity == 0.2f) glass.border_opacity = static_cast<float>(theme_.glass.defaultBorderOpacity);
        if (glass.noise_strength == 0.015f) glass.noise_strength = static_cast<float>(theme_.glass.defaultNoiseStrength);
    }
}

void ThemeResolver::resolveHoverColors(HoverParams& hover) {}

void ThemeResolver::swapTheme(UINode& root, const std::string& new_theme_json) {
    loadTheme(new_theme_json);
    if (loaded_) resolve(root);
}

} // namespace axui
