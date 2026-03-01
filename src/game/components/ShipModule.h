#pragma once
#include <string>
#include <vector>

namespace space {

enum class MountSize { Small, Medium, Large };

using ModuleId = uint16_t;

// ─── Catalogue definition structs (data table, not ECS components) ───────────

struct EngineModuleDef {
  std::string name;
  MountSize size;
  float volumeCost;
  float thrust;
  float rotSpeed;
};

struct WeaponModuleDef {
  std::string name;
  float volumeCost;
  float damage;
  float cooldown;
  float energyCost;
};

struct ShieldModuleDef {
  std::string name;
  float volumeCost;
  float shieldCap;
  float regenRate;
};

struct CargoModuleDef {
  std::string name;
  float volumeCost;
  float capacity;
};

struct PassengerModuleDef {
  std::string name;
  float volumeCost;
  float capacity;
};

struct FuelModuleDef {
  std::string name;
  float volumeCost;
  float capacity;
};

struct PowerModuleDef {
  std::string name;
  float volumeCost;
  float output;
};

// ─── Singleton registry
// ───────────────────────────────────────────────────────
class ModuleRegistry {
public:
  static ModuleRegistry &instance() {
    static ModuleRegistry inst;
    return inst;
  }

  void init(); // populate standard catalogue

  const EngineModuleDef &engine(ModuleId id) const { return engines[id]; }
  const WeaponModuleDef &weapon(ModuleId id) const { return weapons[id]; }
  const ShieldModuleDef &shield(ModuleId id) const { return shields[id]; }
  const CargoModuleDef &cargo(ModuleId id) const { return cargos[id]; }
  const PassengerModuleDef &passenger(ModuleId id) const {
    return passengers[id];
  }
  const FuelModuleDef &fuel(ModuleId id) const { return fuels[id]; }
  const PowerModuleDef &power(ModuleId id) const { return powers[id]; }

  // Stable catalogue IDs — engines
  static constexpr ModuleId ION_THRUSTER_MK1 = 0;
  static constexpr ModuleId FUSION_DRIVE_MK1 = 1;
  static constexpr ModuleId HEAVY_THRUST_MK1 = 2;

  // Stable catalogue IDs — weapons
  static constexpr ModuleId PULSE_CANNON = 0;
  static constexpr ModuleId RAILGUN = 1;

  // Stable catalogue IDs — internals
  static constexpr ModuleId SHIELD_GEN_MK1 = 0;
  static constexpr ModuleId CARGO_BAY = 0;
  static constexpr ModuleId PASSENGER_CABIN = 0;
  static constexpr ModuleId FUEL_TANK = 0;
  static constexpr ModuleId POWER_CORE_MK1 = 0;

  std::vector<EngineModuleDef> engines;
  std::vector<WeaponModuleDef> weapons;
  std::vector<ShieldModuleDef> shields;
  std::vector<CargoModuleDef> cargos;
  std::vector<PassengerModuleDef> passengers;
  std::vector<FuelModuleDef> fuels;
  std::vector<PowerModuleDef> powers;

private:
  ModuleRegistry() = default;
};

} // namespace space
