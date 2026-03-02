#pragma once
#include "components/Economy.h"
#include "components/GameTypes.h"
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
  entt::entity shipEntity;
  uint32_t factionId;
  float price;
};

class EconomyManager {
public:
  static EconomyManager &instance() {
    static EconomyManager inst;
    return inst;
  }

  void init();
  void update(entt::registry &registry, float deltaTime);

  std::vector<ShipOffer> getShipOffers(entt::registry &registry,
                                       entt::entity planet);
  std::map<uint16_t, float> getModuleBids(entt::registry &registry,
                                          entt::entity planet, ProductKey pk);
  std::map<Tier, float> getHullBids(entt::registry &registry,
                                    entt::entity planet);

  bool buyShip(entt::registry &registry, entt::entity planet,
               entt::entity player, Tier sizeTier, b2WorldId worldId);

  bool buyModularShip(entt::registry &registry, entt::entity shipEntity,
                      entt::entity player);

  const Recipe &getRecipe(ProductKey pk) const { return recipes.at(pk); }

private:
  EconomyManager() = default;

  void processProduction(FactionEconomy &fEco, float deltaTime);
  void tryExpandInfrastructure(FactionEconomy &fEco, float deltaTime);
  float calculatePrice(ProductKey pk, float currentStock, float population,
                       bool isAtWar);

  std::map<ProductKey, Recipe> recipes;
  std::vector<ProductKey> productionPriority;
};

} // namespace space
