#pragma once
#include <cstdint>
#include <string>
#include <type_traits>

namespace space {

enum class Tier { T1 = 1, T2 = 2, T3 = 3 };
enum class VisualStyle { Triangle, Square, Circular, Sleek, Polygon };
enum class WeaponType { Energy, Projectile, Missile };

enum class AttributeType {
  Size,
  Thrust,
  Efficiency,
  Mass,
  Caliber,
  ROF,
  Warhead,
  Range,
  Accuracy,
  Capacity,
  Regen,
  Volume,
  Output,
  Guidance, // Missile ammo targeting: T1=unguided, T2=heat-seek, T3=fly-by-wire
  TurnRate  // Reaction Wheel turning power
};

enum class ModuleCategory {
  Engine,
  Weapon,
  Shield,
  Utility,
  Reactor,
  Command,
  Battery,
  Ammo,         // Physical storage rack for ammunition
  ReactionWheel, // Provides TurnRate attribute
  Habitation,    // Capacity for crew and passengers
  Cargo          // Dedicated cargo capacity
};

enum class LayoutPattern { Symmetrical, Radial, Asymmetrical, Alternating };
enum class NacelleStyle { Outriggers, Integrated, Ring, Pods };
enum class HullConnectivity { Monolithic, Skeletal, Modular };

struct ModuleAttribute {
  AttributeType type;
  Tier tier;
  float qualityRoll = 1.0f; // 0.8 to 1.2 for unique stat variance
};

enum class ProductType { Resource, Hull, Module, Ammo };

struct ProductKey {
  ProductType type;
  uint32_t id;          // Resource enum, or catalogue index
  Tier tier = Tier::T1; // Magnitude/Quality tier

  bool operator<(const ProductKey &other) const {
    if (type != other.type)
      return type < other.type;
    if (id != other.id)
      return id < other.id;
    return tier < other.tier;
  }
};

static inline std::string tierName(Tier t) {
  switch (t) {
  case Tier::T1:
    return "Small";
  case Tier::T2:
    return "Medium";
  case Tier::T3:
    return "Large";
  }
  return "Unknown";
}

enum class MissionType { Patrol, Trade, Combat, Expansion, Escort, Piracy };

enum class NamingScheme {
  Raptors,
  Rodents,
  Ungulates,
  Insects,
  Felines,
  Mythical,
  Celestial,
  Avian,
  Geological,
  Meteorological,
  Mechanical
};

using ShipOutfitHash = uint64_t;

} // namespace space
