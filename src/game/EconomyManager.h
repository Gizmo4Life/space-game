#pragma once
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/ShipModule.h"
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <map>
#include <vector>

namespace space {

struct Recipe {
  std::map<ProductKey, float> inputs;
  float laborRequired = 1.0f;
  float baseOutputRate = 1.0f;
};

struct ShipOffer {
  ::entt::entity shipEntity;
  uint32_t factionId;
  float price;
};

struct DetailedHullBid {
  uint32_t factionId;
  Tier tier;
  std::string role;
  float price;
  std::vector<ModuleId> modules;
  HullDef hull;
  std::string hullName;
};

class EconomyManager {
public:
  static EconomyManager &instance() {
    static EconomyManager inst;
    return inst;
  }

  void init();
  void update(::entt::registry &registry, float deltaTime);

  std::vector<ShipOffer> getShipOffers(entt::registry &registry,
                                       entt::entity planet);
  std::map<uint16_t, float> getModuleBids(entt::registry &registry,
                                          entt::entity planet, ProductKey pk);
  std::vector<DetailedHullBid> getHullBids(entt::registry &registry,
                                           entt::entity planet);

  bool buyShip(entt::registry &registry, entt::entity planet,
               entt::entity player, const DetailedHullBid &bid,
               b2WorldId worldId, bool addToFleet = false,
               bool asFlagship = false);

  bool buyModularShip(entt::registry &registry, entt::entity shipEntity,
                      entt::entity player);

  /// Exchange commodity goods between player cargo and planet stockpile.
  /// delta > 0 for player buying from planet, delta < 0 for player selling to
  /// planet.
  bool executeTrade(entt::registry &registry, entt::entity planet,
                    entt::entity player, Resource res, float delta);

  float calculatePrice(ProductKey pk, float currentStock, float population,
                       bool isAtWar);

  const Recipe &getRecipe(ProductKey pk) const { return recipes.at(pk); }

private:
  EconomyManager() = default;

  void processProduction(uint32_t factionId, FactionEconomy &fEco,
                         float deltaTime);
  void tryExpandInfrastructure(uint32_t factionId, FactionEconomy &fEco,
                               float deltaTime);
  void reEvaluateFactionDNA(uint32_t factionId, FactionEconomy &fEco,
                            float deltaTime);
  void reEvaluateTraderLogic(::entt::registry &registry, uint32_t factionId,
                             FactionEconomy &fEco, ::entt::entity currentPlanet,
                             float deltaTime);

  std::map<ProductKey, Recipe> recipes;
  std::vector<ProductKey> productionPriority;
  float accumulationTimer = 0.0f;
};

} // namespace space
