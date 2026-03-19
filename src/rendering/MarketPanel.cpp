#include "MarketPanel.h"
#include "UIUtils.h"
#include "game/EconomyManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/NPCComponent.h"

namespace space {

MarketPanel::MarketPanel(entt::entity planet, entt::entity)
    : planetEntity_(planet) {}

void MarketPanel::handleEvent(const sf::Event &event, const UIContext &ctx,
                               b2WorldId) {
  if (const auto *kp = event.getIf<sf::Event::KeyPressed>()) {
    int maxRes = static_cast<int>(Resource::Refinery);
    if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W) {
      if (selectedMarketIndex_ > 0)
        selectedMarketIndex_--;
    }
    if (kp->code == sf::Keyboard::Key::Down ||
        kp->code == sf::Keyboard::Key::S) {
      if (selectedMarketIndex_ < maxRes)
        selectedMarketIndex_++;
    }
    if (kp->code == sf::Keyboard::Key::Left || kp->code == sf::Keyboard::Key::A) {
      if (selectedQuantity_ > 1)
        selectedQuantity_--;
    }
    if (kp->code == sf::Keyboard::Key::Right || kp->code == sf::Keyboard::Key::D) {
      selectedQuantity_++;
    }

    Resource res = static_cast<Resource>(selectedMarketIndex_);
    if (kp->code == sf::Keyboard::Key::B) {
      EconomyManager::instance().executeTrade(ctx.registry, planetEntity_,
                                               ctx.player, res, (float)selectedQuantity_);
    } else if (kp->code == sf::Keyboard::Key::V) {
      EconomyManager::instance().executeTrade(ctx.registry, planetEntity_,
                                               ctx.player, res, -(float)selectedQuantity_);
    }
  }
}

void MarketPanel::render(sf::RenderTarget &target, const UIContext &ctx,
                         const sf::Font *font, sf::FloatRect rect) {
  if (!font || !ctx.registry.valid(planetEntity_))
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

  dtext("── Commodity Market ──", 18, sf::Color(140, 200, 255));
  y += 10.f;

  if (!ctx.registry.all_of<PlanetEconomy>(planetEntity_)) {
    dtext("No market activity on this body.", 16, sf::Color(200, 200, 200));
    return;
  }

  auto &eco = ctx.registry.get<PlanetEconomy>(planetEntity_);
  CreditsComponent *pCredits =
      ctx.registry.try_get<CreditsComponent>(ctx.player);

  // Aggregated fleet stats
  std::vector<entt::entity> fleetShips;
  fleetShips.push_back(ctx.player);
  auto npcView = ctx.registry.view<NPCComponent>();
  for (auto e : npcView) {
    if (npcView.get<NPCComponent>(e).isPlayerFleet && e != ctx.player)
      fleetShips.push_back(e);
  }

  float totalWeight = 0.f;
  float totalCapacity = 0.f;
  std::map<Resource, float> totalInventory;

  for (auto ship : fleetShips) {
    if (auto *cargo = ctx.registry.try_get<CargoComponent>(ship)) {
      totalWeight += cargo->currentWeight;
      totalCapacity += cargo->maxCapacity;
      for (auto const &[res, qty] : cargo->inventory) {
        totalInventory[res] += qty;
      }
    }
  }

  if (pCredits) {
    dtext("Credits: $" + fmt(pCredits->amount, 0), 16,
          sf::Color(100, 255, 100));
  } else {
    dtext("Credits: N/A", 16, sf::Color(150, 150, 150));
  }

  bool isFull = (totalWeight >= totalCapacity);
  dtext("Fleet Cargo: " + fmt(totalWeight, 0) + " / " + fmt(totalCapacity, 0), 16,
        isFull ? sf::Color::Red : sf::Color::Green);

  y += 5.f;

  int maxRes = static_cast<int>(Resource::Refinery);
  for (int i = 0; i <= maxRes; ++i) {
    Resource res = static_cast<Resource>(i);
    bool sel = (i == selectedMarketIndex_);
    ProductKey pk{ProductType::Resource, (uint32_t)res, Tier::T1};

    float price = eco.currentPrices.count(pk) ? eco.currentPrices.at(pk) : 0.f;
    float stock =
        eco.marketStockpile.count(pk) ? eco.marketStockpile.at(pk) : 0.f;
    float pQty = totalInventory.count(res) ? totalInventory.at(res) : 0.f;

    std::string label = (sel ? "> " : "  ") + getResourceName(res);
    std::string stats = "   Price: $" + fmt(price, 1) +
                        "   Stock: " + fmt(stock, 0) +
                        "   Fleet: " + fmt(pQty, 0);

    sf::Text t1(*font, label, 15);
    t1.setFillColor(sel ? sf::Color::Cyan : sf::Color::White);
    t1.setPosition({x, y});
    target.draw(t1);

    sf::Text t2(*font, stats, 14);
    t2.setFillColor(sf::Color(180, 180, 180));
    t2.setPosition({x + 200.f, y});
    target.draw(t2);

    y += 22.f;
  }

  y = rect.position.y + rect.size.y - 65.f;
  Resource selRes = static_cast<Resource>(selectedMarketIndex_);
  ProductKey selPk{ProductType::Resource, (uint32_t)selRes, Tier::T1};
  float selPrice = eco.currentPrices.count(selPk) ? eco.currentPrices.at(selPk) : 0.f;
  float totalCost = selPrice * selectedQuantity_;

  dtext("Trade Quantity: " + std::to_string(selectedQuantity_) + "   Total Cost: $" + fmt(totalCost, 0), 15, sf::Color::Yellow);
  
  y = rect.position.y + rect.size.y - 30.f;
  dtext("[B] Buy Quantity   [V] Sell Quantity   [Arrows] Navigate / Qty", 13,
        sf::Color(150, 150, 150));
}

} // namespace space
