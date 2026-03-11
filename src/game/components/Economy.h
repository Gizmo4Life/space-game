#pragma once
#include "FactionDNA.h"
#include "GameTypes.h"
#include "HullDef.h"
#include "MissionStats.h"
#include "ShipModule.h"
#include <map>
#include <string>
#include <type_traits>
#include <vector>

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
  Refinery,
  COUNT
};

struct FactionEconomy {
  float populationCount = 0.0f; // In thousands
  std::map<ProductKey, float> stockpile;
  std::map<ProductKey, int> factories;
  std::map<std::pair<Tier, std::string>, int>
      fleetPool;                      // Ready ships by tier and role
  std::vector<ModuleDef> shopModules; // Procedurally generated local modules
  std::vector<AmmoDef> shopAmmo;      // Procedurally generated local ammunition

  std::vector<ModuleDef> scrapyardModules; // Salvaged modules
  std::vector<HullDef> scrapyardHulls;     // Salvaged hulls
  std::vector<ShipBlueprint> parkedShips;  // Physical ships ready for sale

  FactionDNA dna;
  MissionStats stats;
  float credits = 1000.0f;
  bool isAbandoned = false;
};

struct PlanetEconomy {
  std::map<uint32_t, FactionEconomy> factionData;
  std::map<ProductKey, float> marketStockpile; // Aggregate supply for trade
  std::map<ProductKey, float> currentPrices;

  std::vector<ModuleDef> shopModules; // Aggregate planetary module market
  std::vector<AmmoDef> shopAmmo;      // Aggregate planetary ammunition market

  // Scarcity tracking: hull class name -> relative abundance (0.0 to 2.0, 1.0
  // is normal)
  std::map<std::string, float> hullClassScarcity;

  // Base consumption per 1k population (Global reference)
  std::map<Resource, float> baseConsumption;

  float getTotalPopulation() const {
    float total = 0.0f;
    for (auto const &pair : factionData) {
      total += pair.second.populationCount;
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
  case Resource::COUNT:
    break;
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
  case Resource::COUNT:
    break;
  }
  return "?";
}

} // namespace space
