#pragma once
#include "GameTypes.h"
#include <string>
#include <vector>

namespace space {

using ModuleId = uint16_t;
const ModuleId EMPTY_MODULE = 0xFFFF;

struct ModuleDef {
  std::string name;
  ModuleCategory category =
      ModuleCategory::Engine; // Identifies which ECS component owns this
  WeaponType weaponType =
      WeaponType::Energy; // Only meaningful if category == Weapon
  std::vector<ModuleAttribute> attributes;
  float volumeOccupied = 0.f;
  float mass = 0.f;
  float maintenanceCost = 0.f;
  float powerDraw = 0.f; // GW; negative = generation (reactors)

  // Helper to find a specific attribute
  bool hasAttribute(AttributeType type) const {
    for (const auto &attr : attributes)
      if (attr.type == type)
        return true;
    return false;
  }

  Tier getAttributeTier(AttributeType type) const {
    for (const auto &attr : attributes)
      if (attr.type == type)
        return attr.tier;
    return Tier::T1;
  }
};

struct AmmoDef {
  std::string name;
  WeaponType compatibleWeapon = WeaponType::Projectile; // Projectile or Missile
  Tier caliber = Tier::T1; // Must match weapon caliber to use
  Tier warhead = Tier::T1; // T1=Kinetic, T2=Explosive, T3=EMP
  Tier range = Tier::T1;   // Missile-only
  Tier guidance =
      Tier::T1; // Missile-only: T1=unguided, T2=heat-seeking, T3=fly-by-wire
  float massPerRound = 1.0f;
  float volumePerRound = 0.1f;
  float basePrice = 10.0f;

  // For map keys and sorting AmmoStacks
  bool operator<(const AmmoDef &other) const {
    if (compatibleWeapon != other.compatibleWeapon)
      return compatibleWeapon < other.compatibleWeapon;
    if (caliber != other.caliber)
      return caliber < other.caliber;
    if (warhead != other.warhead)
      return warhead < other.warhead;
    return guidance < other.guidance;
  }
};

struct AmmoStack {
  AmmoDef type;
  int count = 0;
};

} // namespace space
