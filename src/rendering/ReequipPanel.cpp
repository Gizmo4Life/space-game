#include "ReequipPanel.h"
#include "UIUtils.h"
#include "game/EconomyManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/GameTypes.h"
#include "game/components/NPCComponent.h"
#include "game/components/ShipStats.h"

namespace space {

ReequipPanel::ReequipPanel(entt::entity planet, entt::entity)
    : planetEntity_(planet) {}

void ReequipPanel::handleEvent(const sf::Event &event, const UIContext &ctx,
                               b2WorldId) {
  if (const auto *kp = event.getIf<sf::Event::KeyPressed>()) {
    if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W) {
      if (targetDays_ < 30)
        targetDays_++;
    }
    if (kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::S) {
      if (targetDays_ > 1)
        targetDays_--;
    }

    if (kp->code == sf::Keyboard::Key::Enter) {
      auto result = EconomyManager::instance().reequipForDuration(
          ctx.registry, planetEntity_, ctx.player, targetDays_);
      foodBought_ = result.foodBought;
      fuelBought_ = result.fuelBought;
      isotopeBought_ = result.isotopeBought;
      totalSpent_ = result.totalSpent;
      statusMessage_ = result.message;
      showResult_ = true;

      // Refresh stats after purchase
      if (ctx.registry.valid(ctx.player) &&
          ctx.registry.all_of<HullDef>(ctx.player)) {
        ShipOutfitter::instance().refreshStats(
            ctx.registry, ctx.player, ctx.registry.get<HullDef>(ctx.player));
      }
    }
  }
}

void ReequipPanel::render(sf::RenderTarget &target, const UIContext &ctx,
                          const sf::Font *font, sf::FloatRect rect) {
  if (!font || !ctx.registry.valid(ctx.player))
    return;

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 20.f;

  auto dtext = [&](const std::string &s, unsigned sz, sf::Color c) {
    sf::Text t(*font, s, sz);
    t.setFillColor(c);
    t.setPosition({x, y});
    target.draw(t);
    y += sz + 6.f;
  };

  dtext("── Reequip for Duration ──", 18, sf::Color(140, 200, 255));
  y += 10.f;

  dtext("Target Duration: " + std::to_string(targetDays_) + " days",
        18, sf::Color::Yellow);
  y += 5.f;

  // Show current stats and needed quantities
  // Aggregate fleet-wide stats
  std::vector<entt::entity> fleetShips;
  fleetShips.push_back(ctx.player);
  auto npcView = ctx.registry.view<NPCComponent>();
  for (auto e : npcView) {
    if (npcView.get<NPCComponent>(e).isPlayerFleet && e != ctx.player)
      fleetShips.push_back(e);
  }

  float totalFoodCons = 0.f, totalFuelCons = 0.f, totalIsoCons = 0.f;
  float totalFoodStock = 0.f, totalFuelStock = 0.f, totalIsoStock = 0.f;
  float totalCargoAvail = 0.f;
  float totalCargoMax = 0.f;

  for (auto ship : fleetShips) {
    if (auto *s = ctx.registry.try_get<ShipStats>(ship)) {
      totalFoodCons += s->foodConsumption;
      totalFuelCons += s->fuelConsumption;
      totalIsoCons += s->isotopesConsumption;
      totalFoodStock += s->foodStock;
      totalFuelStock += s->fuelStock;
      totalIsoStock += s->isotopesStock;
    }
    if (auto *c = ctx.registry.try_get<CargoComponent>(ship)) {
      totalCargoAvail += c->maxCapacity - c->currentWeight;
      totalCargoMax += c->maxCapacity;
    }
  }

  dtext("Fleet Ships: " + std::to_string(fleetShips.size()), 15, sf::Color(180, 220, 255));
  y += 5.f;

  float durationSec = static_cast<float>(targetDays_) * GAME_SECONDS_PER_DAY;

  float foodNeeded = std::max(0.f, totalFoodCons * durationSec - totalFoodStock);
  float fuelNeeded = std::max(0.f, totalFuelCons * durationSec - totalFuelStock);
  float isoNeeded = std::max(0.f, totalIsoCons * durationSec - totalIsoStock);
  float cargoRemaining = totalCargoAvail;

  auto *credits = ctx.registry.try_get<CreditsComponent>(ctx.player);

  if (ctx.registry.all_of<PlanetEconomy>(planetEntity_)) {
    auto &eco = ctx.registry.get<PlanetEconomy>(planetEntity_);

    auto getPrice = [&](Resource res) -> float {
      ProductKey pk{ProductType::Resource, static_cast<uint32_t>(res), Tier::T1};
      return eco.currentPrices.count(pk) ? eco.currentPrices.at(pk) : 0.f;
    };

    auto getStock = [&](Resource res) -> float {
      ProductKey pk{ProductType::Resource, static_cast<uint32_t>(res), Tier::T1};
      return eco.marketStockpile.count(pk) ? eco.marketStockpile.at(pk) : 0.f;
    };

    dtext("Resource     Need     Price    Stock    Est. Cost", 14, sf::Color(180, 180, 180));

    auto showRow = [&](const std::string &name, float needed, Resource res) {
      float price = getPrice(res);
      float stock = getStock(res);
      float buyable = std::min({needed, stock, cargoRemaining});
      if (credits) buyable = std::min(buyable, (price > 0.f) ? credits->amount / price : 999999.f);
      float cost = buyable * price;
      std::string row = name;
      while (row.size() < 13) row += ' ';
      row += fmt(needed, 0);
      while (row.size() < 22) row += ' ';
      row += "$" + fmt(price, 1);
      while (row.size() < 33) row += ' ';
      row += fmt(stock, 0);
      while (row.size() < 42) row += ' ';
      row += "$" + fmt(cost, 0);
      dtext(row, 15, sf::Color::White);
    };

    showRow("Food", foodNeeded, Resource::Food);
    showRow("Fuel", fuelNeeded, Resource::Fuel);
    showRow("Isotopes", isoNeeded, Resource::Isotopes);
  }

  y += 10.f;
  dtext("Fleet Cargo: " + fmt(cargoRemaining, 0) + " / " + fmt(totalCargoMax, 0),
        15, cargoRemaining > 0 ? sf::Color::Green : sf::Color::Red);


  if (credits) {
    dtext("Credits: $" + fmt(credits->amount, 0), 15, sf::Color(100, 255, 100));
  }

  if (showResult_) {
    y += 15.f;
    dtext("── Purchase Summary ──", 16, sf::Color(100, 255, 200));
    if (foodBought_ > 0.f)
      dtext("  Food: +" + fmt(foodBought_, 0), 15, sf::Color::White);
    if (fuelBought_ > 0.f)
      dtext("  Fuel: +" + fmt(fuelBought_, 0), 15, sf::Color::White);
    if (isotopeBought_ > 0.f)
      dtext("  Isotopes: +" + fmt(isotopeBought_, 0), 15, sf::Color::White);
    dtext("  Total Spent: $" + fmt(totalSpent_, 0), 15, sf::Color::Yellow);
    if (!statusMessage_.empty())
      dtext(statusMessage_, 14, sf::Color(255, 200, 100));
  }

  y = rect.position.y + rect.size.y - 30.f;
  dtext("[Up/Down] Adjust Days   [Enter] Purchase   [Esc] Exit", 13,
        sf::Color(150, 150, 150));
}

} // namespace space
