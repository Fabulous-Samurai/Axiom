#pragma once

#include "node.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <stdexcept>
#include <simdjson.h>

namespace axui {

struct ThemeData {
    std::string name;
    std::string displayName;

    std::unordered_map<std::string, Color> colors;
    std::unordered_map<std::string, double> typography;
    std::unordered_map<std::string, std::string> typographyStrings;
    std::unordered_map<std::string, double> spacing;
    std::unordered_map<std::string, double> radius;

    struct ElevationLevel {
        double offset = 0;
        double blur = 0;
        double opacity = 0;
    };
    std::unordered_map<std::string, ElevationLevel> elevation;

    struct GlassDefaults {
        double defaultBlur = 32;
        double defaultOpacity = 0.1;
        double defaultBorderOpacity = 0.2;
        double defaultNoiseStrength = 0.015;
    } glass;

    struct AnimationDefaults {
        double fast = 150;
        double normal = 300;
        double slow = 500;
    } animation;
};

class ThemeResolver {
public:
    void loadTheme(const std::string& theme_json);
    void resolve(UINode& root);
    void swapTheme(UINode& root, const std::string& new_theme_json);

    const ThemeData& currentTheme() const { return theme_; }
    bool isLoaded() const { return loaded_; }
    size_t colorCount() const { return theme_.colors.size(); }
    size_t tokenCount() const;

    Color resolveColor(const std::string& token) const;
    double resolveNumber(const std::string& token) const;
    std::string resolveString(const std::string& token) const;

    bool isToken(const std::string& value) const {
        // Support both "@category.key" and " @category.key" (with leading space)
        if (value.empty()) return false;
        if (value[0] == '@') return true;
        if (value.size() > 1 && value[0] == ' ' && value[1] == '@') return true;
        return false;
    }

    struct TokenParts {
        std::string category;
        std::string key;
    };
    static TokenParts parseToken(const std::string& token);

private:
    ThemeData theme_;
    bool loaded_ = false;
    size_t resolve_count_ = 0;

    void resolveNode(UINode& node);
    void resolveProperties(std::vector<Property>& props);
    void resolveGlassDefaults(GlassParams& glass);
    void resolveHoverColors(HoverParams& hover);
};

} // namespace axui
