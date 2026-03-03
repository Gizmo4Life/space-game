#include "OutfitterPanel.h"
#include "UIUtils.h"
#include "game/EconomyManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/Economy.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
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
        auto &reg = ModuleRegistry::instance();
        if (selectedOutfitterIndex_ >= 0 &&
            selectedOutfitterIndex_ < (int)reg.modules.size()) {
          const auto &mdef = reg.modules[selectedOutfitterIndex_];
          ProductKey pk{ProductType::Module, (uint32_t)selectedOutfitterIndex_,
                        Tier::T1}; // Default T1 for registry lookup

          // For Engines/Weapons, we need a slot. For now, try slot 0.
          // In a full UI, the player would select a slot first.
          ShipOutfitter::instance().refitModule(registry, playerEntity_,
                                                planetEntity_, pk, 0);
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
    const auto &hdef = registry.get<HullDef>(playerEntity_);
    const auto &fdr = registry.get<CreditsComponent>(playerEntity_);

    dtext(hdef.name + " (" + hdef.className + ")", 20, sf::Color::Cyan);
    dtext("Tier: " + tierName(hdef.sizeTier), 14, sf::Color(200, 200, 200));

    // Power & Volume
    float usedVol = 0.f;
    float powerTotal = 0.f;

    auto calcStats = [&](const std::vector<ModuleId> &ids) {
      for (auto id : ids) {
        if (id == EMPTY_MODULE)
          continue;
        const auto &m = ModuleRegistry::instance().getModule(id);
        usedVol += m.volumeOccupied;
        powerTotal -= m.powerDraw;
      }
    };
    if (registry.all_of<InstalledEngines>(playerEntity_))
      calcStats(registry.get<InstalledEngines>(playerEntity_).ids);
    if (registry.all_of<InstalledWeapons>(playerEntity_))
      calcStats(registry.get<InstalledWeapons>(playerEntity_).ids);
    if (registry.all_of<InstalledShields>(playerEntity_))
      calcStats(registry.get<InstalledShields>(playerEntity_).ids);
    if (registry.all_of<InstalledCargo>(playerEntity_))
      calcStats(registry.get<InstalledCargo>(playerEntity_).ids);
    if (registry.all_of<InstalledPower>(playerEntity_))
      calcStats(registry.get<InstalledPower>(playerEntity_).ids);

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

  std::vector<ModuleId> allInstalled;
  auto collect = [&](const std::vector<ModuleId> &ids) {
    for (auto id : ids)
      if (id != EMPTY_MODULE)
        allInstalled.push_back(id);
  };
  if (registry.all_of<InstalledEngines>(playerEntity_))
    collect(registry.get<InstalledEngines>(playerEntity_).ids);
  if (registry.all_of<InstalledWeapons>(playerEntity_))
    collect(registry.get<InstalledWeapons>(playerEntity_).ids);
  if (registry.all_of<InstalledShields>(playerEntity_))
    collect(registry.get<InstalledShields>(playerEntity_).ids);
  if (registry.all_of<InstalledCargo>(playerEntity_))
    collect(registry.get<InstalledCargo>(playerEntity_).ids);
  if (registry.all_of<InstalledPower>(playerEntity_))
    collect(registry.get<InstalledPower>(playerEntity_).ids);

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
      ModuleId modId = allInstalled[i];
      if (modId == EMPTY_MODULE)
        continue;
      const auto &mdef = ModuleRegistry::instance().getModule(modId);

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
  auto &reg = ModuleRegistry::instance();
  for (int i = 0; i < (int)reg.modules.size(); ++i) {
    bool sel = (outfitterMarketMode_ && i == selectedOutfitterIndex_);
    const auto &mdef = reg.modules[i];
    ProductKey pk{ProductType::Module, (uint32_t)i, Tier::T1};

    float price = 0.f;
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
