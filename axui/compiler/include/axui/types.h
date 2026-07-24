#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace axui {

struct Color {
  uint8_t r = 0, g = 0, b = 0, a = 255;

  static Color fromHex(std::string_view hex) {
    Color c;
    if (hex.empty()) return c;
    if (hex[0] == '#') hex.remove_prefix(1);
    if (hex.size() == 6) {
      auto val = std::stoul(std::string(hex), nullptr, 16);
      c.r = (val >> 16) & 0xFF;
      c.g = (val >> 8) & 0xFF;
      c.b = val & 0xFF;
      c.a = 255;
    } else if (hex.size() == 8) {
      auto val = std::stoul(std::string(hex), nullptr, 16);
      c.r = (val >> 24) & 0xFF;
      c.g = (val >> 16) & 0xFF;
      c.b = (val >> 8) & 0xFF;
      c.a = val & 0xFF;
    }
    return c;
  }

  uint32_t toUint32() const {
    return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) |
           (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
  }

  bool operator==(const Color& o) const {
    return r == o.r && g == o.g && b == o.b && a == o.a;
  }
};

enum class Easing : uint8_t {
  Linear = 0,
  InCubic,
  OutCubic,
  InOutCubic,
  InQuad,
  OutQuad,
  InOutQuad,
  OutBack,
  OutElastic,
  OutBounce
};

enum class LayoutType : uint8_t { Column = 0, Row, Grid, Stack, Absolute };

enum class ComponentType : uint8_t {
  Container = 0,
  Row,
  Column,
  Grid,
  GlassPanel,
  KPICard,
  Chart,
  Table,
  Sidebar,
  Navbar,
  Button,
  Gauge,
  Badge,
  Toast,
  Spacer,
  Text,
  Image,
  StatusIndicator,
  ModeBadge,
  ActionMenu
};

inline std::string_view componentTypeName(ComponentType t) {
  switch (t) {
    case ComponentType::Container:
      return "Container";
    case ComponentType::Row:
      return "Row";
    case ComponentType::Column:
      return "Column";
    case ComponentType::Grid:
      return "Grid";
    case ComponentType::GlassPanel:
      return "GlassPanel";
    case ComponentType::KPICard:
      return "KPICard";
    case ComponentType::Chart:
      return "Chart";
    case ComponentType::Table:
      return "Table";
    case ComponentType::Sidebar:
      return "Sidebar";
    case ComponentType::Navbar:
      return "Navbar";
    case ComponentType::Button:
      return "Button";
    case ComponentType::Gauge:
      return "Gauge";
    case ComponentType::Badge:
      return "Badge";
    case ComponentType::Toast:
      return "Toast";
    case ComponentType::Spacer:
      return "Spacer";
    case ComponentType::Text:
      return "Text";
    case ComponentType::Image:
      return "Image";
    case ComponentType::StatusIndicator:
      return "StatusIndicator";
    case ComponentType::ModeBadge:
      return "ModeBadge";
    case ComponentType::ActionMenu:
      return "ActionMenu";
    default:
      return "Container";
  }
}

inline ComponentType componentTypeFromString(std::string_view name) {
  if (name == "Row") return ComponentType::Row;
  if (name == "Column") return ComponentType::Column;
  if (name == "Grid") return ComponentType::Grid;
  if (name == "GlassPanel") return ComponentType::GlassPanel;
  if (name == "KPICard") return ComponentType::KPICard;
  if (name == "Chart") return ComponentType::Chart;
  if (name == "Table") return ComponentType::Table;
  if (name == "Sidebar") return ComponentType::Sidebar;
  if (name == "Navbar") return ComponentType::Navbar;
  if (name == "Button") return ComponentType::Button;
  if (name == "Gauge") return ComponentType::Gauge;
  if (name == "Badge") return ComponentType::Badge;
  if (name == "Toast") return ComponentType::Toast;
  if (name == "Spacer") return ComponentType::Spacer;
  if (name == "Text") return ComponentType::Text;
  if (name == "Image") return ComponentType::Image;
  if (name == "StatusIndicator") return ComponentType::StatusIndicator;
  if (name == "ModeBadge") return ComponentType::ModeBadge;
  if (name == "ActionMenu") return ComponentType::ActionMenu;
  return ComponentType::Container;
}

inline Easing easingFromString(std::string_view name) {
  if (name == "inCubic") return Easing::InCubic;
  if (name == "outCubic") return Easing::OutCubic;
  if (name == "inOutCubic") return Easing::InOutCubic;
  if (name == "inQuad") return Easing::InQuad;
  if (name == "outQuad") return Easing::OutQuad;
  if (name == "inOutQuad") return Easing::InOutQuad;
  if (name == "outBack") return Easing::OutBack;
  if (name == "outElastic") return Easing::OutElastic;
  if (name == "outBounce") return Easing::OutBounce;
  return Easing::Linear;
}

}  // namespace axui
