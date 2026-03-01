#pragma once
#include "game/components/HullDef.h"
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
  Weapons,
  // Infrastructure
  Shipyard,
  Refinery
};

enum class FactionStrategy { Industrial, Trade, Military };

struct FactionEconomy {
  float populationCount = 0.0f; // In thousands
  std::map<Resource, float> stockpile;
  std::map<Resource, int> factories;
  std::map<VesselClass, int> fleetPool; // Ready ships by class
  FactionStrategy strategy = FactionStrategy::Industrial;
  float credits = 1000.0f;
  bool isAbandoned = false;
};

struct PlanetEconomy {
  std::map<uint32_t, FactionEconomy> factionData;
  std::map<Resource, float> marketStockpile; // Aggregate supply for trade
  std::map<Resource, float> currentPrices;

  // Base consumption per 1k population (Global reference)
  std::map<Resource, float> baseConsumption;

  float getTotalPopulation() const {
    float total = 0.0f;
    for (auto const &[id, fEco] : factionData) {
      total += fEco.populationCount;
    }
    return total;
  }
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
  case Resource::Shipyard:
    return "Shipyard";
  case Resource::Refinery:
    return "Refinery";
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
  case Resource::Shipyard:
    return "Sy";
  case Resource::Refinery:
    return "Rf";
  }
  return "?";
}

} // namespace space
