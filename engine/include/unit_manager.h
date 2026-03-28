// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file unit_manager.h
 * @brief Unit conversion and dimensional analysis system
 * Adds support for physical units: meters, seconds, kilograms, etc.
 */
#pragma once

#include "dynamic_calc_types.h"
#include <string_view>
#include "fixed_vector.h"

namespace AXIOM {

enum class UnitType {
    Length, Time, Mass, Temperature, Current, 
    Angle, Area, Volume, Velocity, Acceleration,
    Force, Energy, Power, Pressure, Dimensionless
};

struct Unit {
    UnitType type;
    double scale_factor; // Relative to SI base unit
    std::string_view symbol;
    std::string_view name;
};

#include "axiom_export.h"

class AXIOM_EXPORT UnitManager {
private:
    AXIOM::FixedVector<Unit, 256> units_;
    
public:
    UnitManager();
    
    // Core functionality
    EngineResult ConvertUnit(double value, std::string_view from_unit, std::string_view to_unit) noexcept;
    EngineResult ConvertTemperature(double value, std::string_view from_unit, std::string_view to_unit) noexcept;
    bool AreCompatible(std::string_view unit1, std::string_view unit2) noexcept;
    std::string_view GetCanonicalUnit(UnitType type) noexcept;
    
    // Registration
    void RegisterUnit(std::string_view symbol, UnitType type, double scale, std::string_view name) noexcept;
    AXIOM::FixedVector<std::string_view, 256> GetUnitsOfType(UnitType type) noexcept;
};

} // namespace AXIOM
