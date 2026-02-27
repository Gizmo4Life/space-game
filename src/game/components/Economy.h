#pragma once
#include <map>
#include <string>

namespace space {

enum class Resource {
  // Basic
  Water,
  Crops,
  Hydrocarbons,
  Metals,
  RareMetals,
  Isotopes,
  // Refined
  Food,
  Plastics,
  ManufacturingGoods,
  Electronics,
  Fuel,
  Powercells,
  Weapons
};

struct PlanetEconomy {
  float populationCount = 10.0f; // In thousands
  std::map<Resource, float> stockpile;
  std::map<Resource, int> factories;
  std::map<Resource, float> currentPrices;

  // Base consumption per 1k population
  std::map<Resource, float> baseConsumption;
};

inline std::string getResourceName(Resource res) {
  switch (res) {
  case Resource::Water:
    return "Water";
  case Resource::Crops:
    return "Crops";
  case Resource::Hydrocarbons:
    return "Hydrocarbons";
  case Resource::Metals:
    return "Metals";
  case Resource::RareMetals:
    return "Rare Metals";
  case Resource::Isotopes:
    return "Isotopes";
  case Resource::Food:
    return "Food";
  case Resource::Plastics:
    return "Plastics";
  case Resource::ManufacturingGoods:
    return "Mfg Goods";
  case Resource::Electronics:
    return "Electronics";
  case Resource::Fuel:
    return "Fuel";
  case Resource::Powercells:
    return "Powercells";
  case Resource::Weapons:
    return "Weapons";
  }
  return "Unknown";
}

inline std::string getResourceInitial(Resource res) {
  switch (res) {
  case Resource::Water:
    return "W";
  case Resource::Crops:
    return "C";
  case Resource::Hydrocarbons:
    return "H";
  case Resource::Metals:
    return "M";
  case Resource::RareMetals:
    return "R";
  case Resource::Isotopes:
    return "I";
  case Resource::Food:
    return "Fd";
  case Resource::Plastics:
    return "Pl";
  case Resource::ManufacturingGoods:
    return "Mg";
  case Resource::Electronics:
    return "El";
  case Resource::Fuel:
    return "Fl";
  case Resource::Powercells:
    return "Pc";
  case Resource::Weapons:
    return "Wp";
  }
  return "?";
}

} // namespace space
