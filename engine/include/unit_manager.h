// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file unit_manager.h
 * @brief Unit conversion and dimensional analysis system
 * Adds support for physical units: meters, seconds, kilograms, etc.
 */
#pragma once

#include "dynamic_calc_types.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace AXIOM {

enum class UnitType {
    Length, Time, Mass, Temperature, Current, 
    Angle, Area, Volume, Velocity, Acceleration,
    Force, Energy, Power, Pressure, Dimensionless
};

struct Unit {
    UnitType type;
    double scale_factor; // Relative to SI base unit
    std::string symbol;
    std::string name;
};

#include "axiom_export.h"

class AXIOM_EXPORT UnitManager {
private:
    std::unordered_map<std::string, Unit> units_;
    
public:
    UnitManager();
    
    // Core functionality
    EngineResult ConvertUnit(double value, const std::string& from_unit, const std::string& to_unit);
    EngineResult ConvertTemperature(double value, const std::string& from_unit, const std::string& to_unit);
    bool AreCompatible(const std::string& unit1, const std::string& unit2);
    std::string GetCanonicalUnit(UnitType type);
    
    // Registration
    void RegisterUnit(const std::string& symbol, UnitType type, double scale, const std::string& name);
    std::vector<std::string> GetUnitsOfType(UnitType type);
};

} // namespace AXIOM
