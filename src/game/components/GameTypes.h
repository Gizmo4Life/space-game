#pragma once
#include <cstdint>
#include <string>
#include <type_traits>

namespace space {

enum class Tier { T1 = 1, T2 = 2, T3 = 3, T4 = 4 };
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
  Maintenance
};

struct ModuleAttribute {
  AttributeType type;
  Tier tier;
};

enum class ProductType { Resource, Hull, Module };

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
    return "Light";
  case Tier::T2:
    return "Medium";
  case Tier::T3:
    return "Heavy";
  case Tier::T4:
    return "Capital";
  }
  return "Unknown";
}

} // namespace space
