// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "unit_manager.h"
#include <numbers>
#include <algorithm>

namespace AXIOM {

UnitManager::UnitManager() {
    // Length units
    RegisterUnit("m", UnitType::Length, 1.0, "meter");
    RegisterUnit("km", UnitType::Length, 1000.0, "kilometer");
    RegisterUnit("cm", UnitType::Length, 0.01, "centimeter");
    RegisterUnit("mm", UnitType::Length, 0.001, "millimeter");
    RegisterUnit("ft", UnitType::Length, 0.3048, "foot");
    RegisterUnit("in", UnitType::Length, 0.0254, "inch");
    RegisterUnit("yd", UnitType::Length, 0.9144, "yard");
    RegisterUnit("mi", UnitType::Length, 1609.344, "mile");

    // Mass units
    RegisterUnit("kg", UnitType::Mass, 1.0, "kilogram");
    RegisterUnit("g", UnitType::Mass, 0.001, "gram");
    RegisterUnit("lb", UnitType::Mass, 0.453592, "pound");
    RegisterUnit("oz", UnitType::Mass, 0.0283495, "ounce");
    RegisterUnit("t", UnitType::Mass, 1000.0, "metric ton");

    // Time units
    RegisterUnit("s", UnitType::Time, 1.0, "second");
    RegisterUnit("min", UnitType::Time, 60.0, "minute");
    RegisterUnit("h", UnitType::Time, 3600.0, "hour");
    RegisterUnit("day", UnitType::Time, 86400.0, "day");
    RegisterUnit("week", UnitType::Time, 604800.0, "week");
    RegisterUnit("year", UnitType::Time, 31557600.0, "year");

    // Temperature units (special handling needed)
    RegisterUnit("K", UnitType::Temperature, 1.0, "kelvin");
    RegisterUnit("C", UnitType::Temperature, 1.0, "celsius");
    RegisterUnit("F", UnitType::Temperature, 1.0, "fahrenheit");

    // Angle units
    RegisterUnit("rad", UnitType::Angle, 1.0, "radian");
    RegisterUnit("deg", UnitType::Angle, std::numbers::pi/180.0, "degree");
    RegisterUnit("grad", UnitType::Angle, std::numbers::pi/200.0, "gradian");

    // Sort for binary search
    std::ranges::sort(units_, {}, &Unit::symbol);
}

void UnitManager::RegisterUnit(std::string_view symbol, UnitType type, double scale, std::string_view name) noexcept {
    units_.emplace_back(type, scale, symbol, name);
}

EngineResult UnitManager::ConvertUnit(double value, std::string_view from_unit, std::string_view to_unit) noexcept {
    auto from_it = std::ranges::lower_bound(units_, from_unit, {}, &Unit::symbol);
    auto to_it = std::ranges::lower_bound(units_, to_unit, {}, &Unit::symbol);

    if (from_it == units_.end() || from_it->symbol != from_unit || 
        to_it == units_.end() || to_it->symbol != to_unit) {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }

    if (from_it->type != to_it->type) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    // Special handling for temperature
    if (from_it->type == UnitType::Temperature) {
        return ConvertTemperature(value, from_unit, to_unit);
    }

    // Standard linear conversion: convert to base unit, then to target unit
    double base_value = value * from_it->scale_factor;
    double result = base_value / to_it->scale_factor;

    return CreateSuccessResult(result);
}

AXIOM::EngineResult UnitManager::ConvertTemperature(double value, std::string_view from_unit, std::string_view to_unit) noexcept {
    // Convert to Kelvin first
    double kelvin;
    if (from_unit == "C") {
        kelvin = value + 273.15;
    } else if (from_unit == "F") {
        kelvin = (value - 32.0) * 5.0/9.0 + 273.15;
    } else if (from_unit == "K") {
        kelvin = value;
    } else {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }

    // Convert from Kelvin to target
    double result;
    if (to_unit == "K") {
        result = kelvin;
    } else if (to_unit == "C") {
        result = kelvin - 273.15;
    } else if (to_unit == "F") {
        result = (kelvin - 273.15) * 9.0/5.0 + 32.0;
    } else {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }

    return CreateSuccessResult(result);
}

bool UnitManager::AreCompatible(std::string_view unit1, std::string_view unit2) noexcept {
    auto u1_it = std::ranges::lower_bound(units_, unit1, {}, &Unit::symbol);
    auto u2_it = std::ranges::lower_bound(units_, unit2, {}, &Unit::symbol);

    return (u1_it != units_.end() && u1_it->symbol == unit1 && 
            u2_it != units_.end() && u2_it->symbol == unit2 && 
            u1_it->type == u2_it->type);
}

std::string_view UnitManager::GetCanonicalUnit(UnitType type) noexcept {
    switch (type) {
        case UnitType::Length: return "m";
        case UnitType::Mass: return "kg";
        case UnitType::Time: return "s";
        case UnitType::Temperature: return "K";
        case UnitType::Angle: return "rad";
        default: return "";
    }
}

AXIOM::FixedVector<std::string_view, 256> UnitManager::GetUnitsOfType(UnitType type) noexcept {
    AXIOM::FixedVector<std::string_view, 256> result;
    for (const auto& [u_type, scale, symbol, name] : units_) {
        if (u_type == type) {
            result.push_back(symbol);
        }
    }
    return result;
}

} // namespace AXIOM
