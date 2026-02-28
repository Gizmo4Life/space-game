#pragma once
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include <entt/entt.hpp>

namespace space {

class TradeManager {
public:
  static bool buy(entt::registry &registry, entt::entity buyer,
                  entt::entity planet, Resource res, float amount) {
    if (!registry.all_of<CargoComponent, CreditsComponent>(buyer))
      return false;
    if (!registry.all_of<PlanetEconomy>(planet))
      return false;

    auto &cargo = registry.get<CargoComponent>(buyer);
    auto &credits = registry.get<CreditsComponent>(buyer);
    auto &eco = registry.get<PlanetEconomy>(planet);

    if (eco.currentPrices.find(res) == eco.currentPrices.end())
      return false;

    float price = eco.currentPrices[res];
    float totalCost = price * amount;

    if (credits.amount < totalCost)
      return false;
    if (eco.marketStockpile[res] < amount)
      return false;
    if (!cargo.add(res, amount))
      return false;

    credits.amount -= totalCost;
    eco.marketStockpile[res] -= amount;

    return true;
  }

  static bool sell(entt::registry &registry, entt::entity seller,
                   entt::entity planet, Resource res, float amount) {
    if (!registry.all_of<CargoComponent, CreditsComponent>(seller))
      return false;
    if (!registry.all_of<PlanetEconomy>(planet))
      return false;

    auto &cargo = registry.get<CargoComponent>(seller);
    auto &credits = registry.get<CreditsComponent>(seller);
    auto &eco = registry.get<PlanetEconomy>(planet);

    if (eco.currentPrices.find(res) == eco.currentPrices.end())
      return false;

    float price = eco.currentPrices[res] * 0.9f; // Buyback at 90%
    float totalProfit = price * amount;

    if (!cargo.remove(res, amount))
      return false;

    credits.amount += totalProfit;
    eco.marketStockpile[res] += amount;

    return true;
  }
};

} // namespace space
