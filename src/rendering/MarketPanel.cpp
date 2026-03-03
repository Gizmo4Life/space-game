#include "MarketPanel.h"
#include "UIUtils.h"
#include "game/EconomyManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"

namespace space {

MarketPanel::MarketPanel(entt::entity planet, entt::entity player)
    : planetEntity_(planet), playerEntity_(player) {}

void MarketPanel::handleEvent(const sf::Event &event, entt::registry &registry,
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

    Resource res = static_cast<Resource>(selectedMarketIndex_);
    if (kp->code == sf::Keyboard::Key::B) {
      EconomyManager::instance().executeTrade(registry, planetEntity_,
                                              playerEntity_, res, 1.0f);
    } else if (kp->code == sf::Keyboard::Key::V) {
      EconomyManager::instance().executeTrade(registry, planetEntity_,
                                              playerEntity_, res, -1.0f);
    }
  }
}

void MarketPanel::render(sf::RenderWindow &window, entt::registry &registry,
                         const sf::Font *font, sf::FloatRect rect) {
  if (!font || !registry.valid(planetEntity_))
    return;

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 20.f;

  auto dtext = [&](const std::string &s, unsigned sz, sf::Color c) {
    sf::Text t(*font, s, sz);
    t.setFillColor(c);
    t.setPosition({x, y});
    window.draw(t);
    y += sz + 6.f;
  };

  dtext("── Commodity Market ──", 18, sf::Color(140, 200, 255));
  y += 10.f;

  if (!registry.all_of<PlanetEconomy>(planetEntity_)) {
    dtext("No market activity on this body.", 16, sf::Color(200, 200, 200));
    return;
  }

  auto &eco = registry.get<PlanetEconomy>(planetEntity_);
  CargoComponent *pCargo = registry.try_get<CargoComponent>(playerEntity_);
  CreditsComponent *pCredits =
      registry.try_get<CreditsComponent>(playerEntity_);

  if (pCredits) {
    dtext("Credits: $" + fmt(pCredits->amount, 0), 16,
          sf::Color(100, 255, 100));
  } else {
    dtext("Credits: N/A", 16, sf::Color(150, 150, 150));
  }
  y += 5.f;

  int maxRes = static_cast<int>(Resource::Refinery);
  for (int i = 0; i <= maxRes; ++i) {
    Resource res = static_cast<Resource>(i);
    bool sel = (i == selectedMarketIndex_);
    ProductKey pk{ProductType::Resource, (uint32_t)res, Tier::T1};

    float price = eco.currentPrices.count(pk) ? eco.currentPrices.at(pk) : 0.f;
    float stock =
        eco.marketStockpile.count(pk) ? eco.marketStockpile.at(pk) : 0.f;
    float pQty = (pCargo && pCargo->inventory.count(res))
                     ? pCargo->inventory.at(res)
                     : 0.f;

    std::string label = (sel ? "> " : "  ") + getResourceName(res);
    std::string stats = "   Price: $" + fmt(price, 1) +
                        "   Stock: " + fmt(stock, 0) +
                        "   Cargo: " + fmt(pQty, 0);

    sf::Text t1(*font, label, 15);
    t1.setFillColor(sel ? sf::Color::Cyan : sf::Color::White);
    t1.setPosition({x, y});
    window.draw(t1);

    sf::Text t2(*font, stats, 14);
    t2.setFillColor(sf::Color(180, 180, 180));
    t2.setPosition({x + 200.f, y});
    window.draw(t2);

    y += 22.f;
  }

  y = rect.position.y + rect.size.y - 40.f;
  dtext("[B] Buy 1   [V] Sell 1   [W/S] Navigate", 13,
        sf::Color(150, 150, 150));
}

} // namespace space
