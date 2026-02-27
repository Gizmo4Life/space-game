#pragma once
#include <map>
#include <string>

namespace space {

enum class RawResource { MetalOre, Hydrocarbons, Silicates, BioMatter };

enum class RefinedGood { Plasteel, FusionFuel, Electronics, NutritionSlurry };

struct PlanetEconomy {
  // Abundance multipliers for raw harvesting (0.0 to 1.0)
  std::map<RawResource, float> rawAbundance;

  // Stockpile of refined goods
  std::map<RefinedGood, float> stockpile;

  // Growth/Refinement stats
  float infrastructureLevel = 1.0f;
  float populationCount = 1000.0f;

  // Market prices for refined goods
  std::map<RefinedGood, float> currentPrices;
};

// Helper to convert Raw -> Refined mapping
inline RefinedGood getRefinedFromRaw(RawResource raw) {
  switch (raw) {
  case RawResource::MetalOre:
    return RefinedGood::Plasteel;
  case RawResource::Hydrocarbons:
    return RefinedGood::FusionFuel;
  case RawResource::Silicates:
    return RefinedGood::Electronics;
  case RawResource::BioMatter:
    return RefinedGood::NutritionSlurry;
  }
  return RefinedGood::Plasteel;
}

inline std::string getGoodName(RefinedGood good) {
  switch (good) {
  case RefinedGood::Plasteel:
    return "Plasteel";
  case RefinedGood::FusionFuel:
    return "Fuel";
  case RefinedGood::Electronics:
    return "Electronics";
  case RefinedGood::NutritionSlurry:
    return "Food";
  }
  return "Unknown";
}

inline std::string getGoodInitial(RefinedGood good) {
  switch (good) {
  case RefinedGood::Plasteel:
    return "P";
  case RefinedGood::FusionFuel:
    return "F";
  case RefinedGood::Electronics:
    return "E";
  case RefinedGood::NutritionSlurry:
    return "S";
  }
  return "?";
}

} // namespace space
