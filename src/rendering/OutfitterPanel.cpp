#include "OutfitterPanel.h"
#include "UIUtils.h"
#include "game/EconomyManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/Economy.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/NPCComponent.h"
#include "game/components/ShipModule.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

namespace space {

OutfitterPanel::OutfitterPanel(entt::entity planet, entt::entity player)
    : planetEntity_(planet), playerEntity_(player) {}

void OutfitterPanel::handleEvent(const sf::Event &event,
                                 ::entt::registry &registry, b2WorldId) {
  if (const auto *kp = event.getIf<sf::Event::KeyPressed>()) {
    if (kp->code == sf::Keyboard::Key::Tab) {
      outfitterMarketMode_ = !outfitterMarketMode_;
      selectedOutfitterIndex_ = 0;
    }

    // Cycle target ship
    if (kp->code == sf::Keyboard::Key::Left ||
        kp->code == sf::Keyboard::Key::Right) {
      std::vector<entt::entity> fleet;
      if (registry.valid(playerEntity_))
        fleet.push_back(playerEntity_);
      auto npcView = registry.view<NPCComponent>();
      for (auto e : npcView) {
        if (npcView.get<NPCComponent>(e).isPlayerFleet) {
          fleet.push_back(e);
        }
      }

      if (!fleet.empty()) {
        if (targetShip_ == entt::null)
          targetShip_ = playerEntity_;
        auto it = std::find(fleet.begin(), fleet.end(), targetShip_);
        int idx = (it != fleet.end()) ? std::distance(fleet.begin(), it) : 0;

        if (kp->code == sf::Keyboard::Key::Left) {
          idx = (idx - 1 + fleet.size()) % fleet.size();
        } else {
          idx = (idx + 1) % fleet.size();
        }
        targetShip_ = fleet[idx];
      }
    }
    if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W) {
      if (selectedOutfitterIndex_ > 0)
        selectedOutfitterIndex_--;
    }
    if (kp->code == sf::Keyboard::Key::Down ||
        kp->code == sf::Keyboard::Key::S) {
      selectedOutfitterIndex_++;
    }
    if (kp->code == sf::Keyboard::Key::Enter ||
        kp->code == sf::Keyboard::Key::B) {
      if (outfitterMarketMode_) {
        std::vector<ModuleDef> shopModules;
        if (registry.all_of<PlanetEconomy>(planetEntity_)) {
          shopModules = registry.get<PlanetEconomy>(planetEntity_).shopModules;
        }
        if (selectedOutfitterIndex_ >= 0 &&
            selectedOutfitterIndex_ < (int)shopModules.size()) {
          const auto &mdef = shopModules[selectedOutfitterIndex_];
          ProductKey pk{ProductType::Module, (uint32_t)selectedOutfitterIndex_,
                        Tier::T1}; // Default T1 for registry lookup

          // For Engines/Weapons, we need a slot. For now, try slot 0.
          // In a full UI, the player would select a slot first.
          entt::entity target =
              registry.valid(targetShip_) ? targetShip_ : playerEntity_;
          ShipOutfitter::instance().refitModule(registry, target, planetEntity_,
                                                pk, 0);
        }
      }
    }
  }
}

void OutfitterPanel::render(sf::RenderWindow &window,
                            ::entt::registry &registry, const sf::Font *font,
                            sf::FloatRect rect) {
  if (!font || !registry.valid(playerEntity_))
    return;

  if (!registry.valid(targetShip_)) {
    targetShip_ = playerEntity_;
  }

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 20.f;

  auto dtext = [&](const std::string &s, unsigned sz, sf::Color c) {
    sf::Text t(*font, s, sz);
    t.setFillColor(c);
    t.setPosition({x, y});
    window.draw(t);
    y += sz + 6.f;
  };

  dtext("── Vessel Outfitter ──", 18, sf::Color(140, 200, 255));
  CreditsComponent *pCredits =
      registry.try_get<CreditsComponent>(playerEntity_);
  if (pCredits) {
    // Assuming hdef and fdr are available in this scope,
    // or need to be fetched from the playerEntity_
    // For now, I'll assume they are available as per the instruction's context.
    // If not, this would cause a compilation error.
    const auto &hdef = registry.get<HullDef>(targetShip_);
    const auto &fdr = registry.get<CreditsComponent>(playerEntity_);

    dtext(hdef.name + " (" + hdef.className + ") [Left/Right to Swap Ship]", 20,
          sf::Color::Cyan);
    dtext("Tier: " + tierName(hdef.sizeTier), 14, sf::Color(200, 200, 200));

    // Power & Volume
    float usedVol = 0.f;
    float powerTotal = 0.f;

    auto calcStats = [&](const std::vector<ModuleDef> &modules) {
      for (const auto &m : modules) {
        if (m.name.empty() || m.name == "Empty")
          continue;
        usedVol += m.volumeOccupied;
        powerTotal -= m.powerDraw;
      }
    };
    if (registry.all_of<InstalledEngines>(targetShip_))
      calcStats(registry.get<InstalledEngines>(targetShip_).modules);
    if (registry.all_of<InstalledWeapons>(targetShip_))
      calcStats(registry.get<InstalledWeapons>(targetShip_).modules);
    if (registry.all_of<InstalledShields>(targetShip_))
      calcStats(registry.get<InstalledShields>(targetShip_).modules);
    if (registry.all_of<InstalledCargo>(targetShip_))
      calcStats(registry.get<InstalledCargo>(targetShip_).modules);
    if (registry.all_of<InstalledPower>(targetShip_))
      calcStats(registry.get<InstalledPower>(targetShip_).modules);

    dtext("Volume: " + fmt(usedVol, 1) + "/" + fmt(hdef.internalVolume, 0) +
              " cubic m",
          14, usedVol > hdef.internalVolume ? sf::Color::Red : sf::Color::Cyan);
    dtext("Power: " + fmt(powerTotal, 1) + " GW", 14,
          powerTotal < 0 ? sf::Color::Red : sf::Color(100, 255, 100));

    // Slots summary
    y += 5.f;
    sf::Text slotSum(*font, hdef.getSlotSummary(), 12);
    slotSum.setFillColor(sf::Color(150, 150, 150));
    slotSum.setPosition({x, y});
    window.draw(slotSum);
    y += 35.f;

    dtext("Credits: $" + fmt(fdr.amount, 0), 16, sf::Color(100, 255, 100));
  } else {
    dtext("Credits: N/A", 16, sf::Color(150, 150, 150));
  }
  y += 10.f;

  std::vector<ModuleDef> allInstalled;
  auto collect = [&](const std::vector<ModuleDef> &modules) {
    for (const auto &m : modules)
      if (!m.name.empty() && m.name != "Empty")
        allInstalled.push_back(m);
  };
  if (registry.all_of<InstalledEngines>(targetShip_))
    collect(registry.get<InstalledEngines>(targetShip_).modules);
  if (registry.all_of<InstalledWeapons>(targetShip_))
    collect(registry.get<InstalledWeapons>(targetShip_).modules);
  if (registry.all_of<InstalledShields>(targetShip_))
    collect(registry.get<InstalledShields>(targetShip_).modules);
  if (registry.all_of<InstalledCargo>(targetShip_))
    collect(registry.get<InstalledCargo>(targetShip_).modules);
  if (registry.all_of<InstalledPower>(targetShip_))
    collect(registry.get<InstalledPower>(targetShip_).modules);

  // Left: Player Modules
  float leftX = x;
  float rightX = rect.position.x + 400.f;
  float startY = y;

  dtext("Installed Modules", 15, sf::Color::Yellow);
  if (allInstalled.empty()) {
    dtext("Empty", 14, sf::Color(150, 150, 150));
  } else {
    for (int i = 0; i < (int)allInstalled.size(); ++i) {
      bool sel = (!outfitterMarketMode_ && i == selectedOutfitterIndex_);
      const auto &mdef = allInstalled[i];

      std::string label = (sel ? "> " : "  ") + mdef.name;
      sf::Text t(*font, label, 14);
      t.setFillColor(sel ? sf::Color::Cyan : sf::Color::White);
      t.setPosition({leftX, y});
      window.draw(t);
      y += 18.f;

      std::string statsLine = "    Vol: " + fmt(mdef.volumeOccupied, 1) +
                              "  Mass: " + fmt(mdef.mass, 1) +
                              "  Power: " + fmt(mdef.powerDraw, 1) + " GW";
      sf::Text st(*font, statsLine, 12);
      st.setFillColor(sf::Color(160, 160, 160));
      st.setPosition({leftX, y});
      window.draw(st);
      y += 18.f;

      if (sel) {
        for (const auto &attr : mdef.attributes) {
          sf::Text labelText(*font, "    " + getAttributeName(attr.type) + ": ",
                             12);
          labelText.setFillColor(sf::Color(180, 180, 180));
          labelText.setPosition({leftX, y});
          window.draw(labelText);

          sf::Text starText(*font, getTierStars(attr.tier), 12);
          starText.setFillColor(sf::Color::Yellow);
          starText.setPosition({leftX + 130.f, y});
          window.draw(starText);

          y += 16.f;
        }
      }
    }
  }

  // Right: Market Modules
  y = startY;
  x = rightX;
  dtext("Planet Market", 15, sf::Color::Yellow);

  // For now, list global registry modules as a test market
  std::vector<ModuleDef> shopModules;
  if (registry.all_of<PlanetEconomy>(planetEntity_)) {
    shopModules = registry.get<PlanetEconomy>(planetEntity_).shopModules;
  }
  for (int i = 0; i < (int)shopModules.size(); ++i) {
    bool sel = (outfitterMarketMode_ && i == selectedOutfitterIndex_);
    const auto &mdef = shopModules[i];
    ProductKey pk{ProductType::Module, (uint32_t)i, Tier::T1}; // Placeholder

    float price = 500.f; // Base Price
    if (registry.all_of<PlanetEconomy>(planetEntity_)) {
      auto &eco = registry.get<PlanetEconomy>(planetEntity_);
      price = EconomyManager::instance().calculatePrice(
          pk, eco.marketStockpile[pk], eco.getTotalPopulation(), false);
    }

    std::string label = (sel ? "> " : "  ") + mdef.name + " $" + fmt(price, 0);
    sf::Text t(*font, label, 14);
    t.setFillColor(sel ? sf::Color::Cyan : sf::Color::White);
    t.setPosition({x, y});
    window.draw(t);
    y += 18.f;

    std::string statsLine = "    Vol: " + fmt(mdef.volumeOccupied, 1) +
                            "  Mass: " + fmt(mdef.mass, 1) +
                            "  Power: " + fmt(mdef.powerDraw, 1) + " GW";
    sf::Text st(*font, statsLine, 12);
    st.setFillColor(sf::Color(160, 160, 160));
    st.setPosition({x, y});
    window.draw(st);
    y += 18.f;

    if (sel) {
      for (const auto &attr : mdef.attributes) {
        sf::Text labelText(*font, "    " + getAttributeName(attr.type) + ": ",
                           12);
        labelText.setFillColor(sf::Color(180, 180, 180));
        labelText.setPosition({x, y});
        window.draw(labelText);

        sf::Text starText(*font, getTierStars(attr.tier), 12);
        starText.setFillColor(sf::Color::Yellow);
        starText.setPosition({x + 130.f, y});
        window.draw(starText);
        y += 16.f;
      }
    }
  }

  x = rect.position.x + 20.f;
  y = rect.position.y + rect.size.y - 40.f;
  dtext("[Tab] Switch Panel   [Enter/B] Buy/Sell   [W/S] Navigate", 13,
        sf::Color(150, 150, 150));
}

} // namespace space
