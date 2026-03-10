#include "OutfitterPanel.h"
#include "ShipRenderer.h"
#include "UIUtils.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/NPCComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

namespace space {

OutfitterPanel::OutfitterPanel(entt::entity planet, entt::entity player)
    : planetEntity_(planet), playerEntity_(player) {}

void OutfitterPanel::handleEvent(const sf::Event &event,
                                 entt::registry &registry, b2WorldId) {
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
      auto npcView = registry.template view<NPCComponent>();
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
    if (kp->code == sf::Keyboard::Key::Z) {
      outfitterTab_ = (outfitterTab_ + 1) % 2;
    }
    if (kp->code == sf::Keyboard::Key::Enter ||
        kp->code == sf::Keyboard::Key::B) {
      if (outfitterMarketMode_) {
        uint32_t primaryFactionId = 0;
        if (registry.all_of<PlanetEconomy>(planetEntity_)) {
          auto &eco = registry.get<PlanetEconomy>(planetEntity_);
          float maxPop = -1.f;
          for (auto const &entry : eco.factionData) {
            uint32_t id = entry.first;
            auto const &fEco = entry.second;
            if (fEco.populationCount > maxPop) {
              maxPop = fEco.populationCount;
              primaryFactionId = id;
            }
          }
        }

        // Module Tab
        if (outfitterTab_ == 0) {
          std::vector<ModuleDef> shopModules;
          if (primaryFactionId != 0) {
            auto &eco = registry.get<PlanetEconomy>(
                planetEntity_); // Re-get eco if needed, or pass
                                // primaryFactionId
            shopModules = eco.factionData[primaryFactionId].shopModules;
          }
          if (selectedOutfitterIndex_ >= 0 &&
              selectedOutfitterIndex_ < (int)shopModules.size()) {
            ProductKey pk{ProductType::Module,
                          (uint32_t)selectedOutfitterIndex_, Tier::T1};
            entt::entity target =
                registry.valid(targetShip_) ? targetShip_ : playerEntity_;
            ShipOutfitter::instance().refitModule(registry, target,
                                                  planetEntity_, pk, 0);
          }
        } else {
          // Ammo Tab
          std::vector<AmmoDef> shopAmmo;
          if (registry.all_of<PlanetEconomy>(planetEntity_)) {
            auto &eco = registry.get<PlanetEconomy>(planetEntity_);
            uint32_t fid = 0;
            float maxPop = -1.f;
            for (auto const &entry : eco.factionData) {
              uint32_t id = entry.first;
              auto const &fEco = entry.second;
              if (fEco.populationCount > maxPop) {
                maxPop = fEco.populationCount;
                fid = id;
              }
            }
            if (fid != 0)
              shopAmmo = eco.factionData[fid].shopAmmo;
          }
          if (selectedOutfitterIndex_ >= 0 &&
              selectedOutfitterIndex_ < (int)shopAmmo.size()) {
            // TODO: call ShipOutfitter ammo refit when implemented
          }
        }
      }
    }
  }
}

void OutfitterPanel::render(sf::RenderTarget &target, entt::registry &registry,
                            const sf::Font *font, sf::FloatRect rect) {
  if (!font || !registry.valid(playerEntity_))
    return;

  // Update playerEntity_ in case a Flagship was purchased in the Shipyard
  auto playerView = registry.template view<PlayerComponent>();
  for (auto e : playerView) {
    if (playerView.get<PlayerComponent>(e).isFlagship) {
      playerEntity_ = e;
      break;
    }
  }

  if (!registry.valid(targetShip_)) {
    targetShip_ = playerEntity_;
  }

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 20.f;

  auto dtext = [&](const std::string &s, unsigned sz, sf::Color c) {
    sf::Text t(*font, s, sz);
    t.setFillColor(c);
    t.setPosition({x, y});
    target.draw(t);
    y += sz + 6.f;
  };

  dtext("── Vessel Outfitter ──", 18, sf::Color(140, 200, 255));
  CreditsComponent *pCredits =
      registry.try_get<CreditsComponent>(playerEntity_);
  if (pCredits) {
    if (registry.all_of<HullDef>(targetShip_)) {
      const auto &hdef = registry.get<HullDef>(targetShip_);

      dtext(hdef.name + " (" + hdef.className + ") [Left/Right to Swap Ship]",
            20, sf::Color::Cyan);
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
            14,
            usedVol > hdef.internalVolume ? sf::Color::Red : sf::Color::Cyan);
      dtext("Power: " + fmt(powerTotal, 1) + " GW", 14,
            powerTotal < 0 ? sf::Color::Red : sf::Color(100, 255, 100));

      // Slots summary
      y += 5.f;
      sf::Text slotSum(*font, hdef.getSlotSummary(), 12);
      slotSum.setFillColor(sf::Color(150, 150, 150));
      slotSum.setPosition({x, y});
      target.draw(slotSum);
      y += 35.f;
    } else {
      dtext("No Active Vessel [Left/Right to Swap Ship]", 20, sf::Color::Cyan);
      y += 50.f;
    }

    dtext("Credits: $" + fmt(pCredits->amount, 0), 16,
          sf::Color(100, 255, 100));

    // Ship Blueprint Preview
    const auto &hdef = registry.get<HullDef>(targetShip_);
    uint32_t factionId = 0;
    if (registry.all_of<Faction>(targetShip_)) {
      factionId = registry.get<Faction>(targetShip_).getMajorityFaction();
    }
    const auto &faction = FactionManager::instance().getFaction(factionId);

    sf::Vector2f previewPos = {rect.position.x + 280.f,
                               rect.position.y + 100.f};
    ShipRenderParams sparams;
    sparams.mode = RenderMode::Schematic;
    sparams.color = faction.color;
    sparams.scale = 1.0f;
    sparams.viewScale = 40.0f; // Scale for Outfitter layout
    ShipRenderer::drawShip(target, hdef, previewPos, sparams);
  } else {
    dtext("Credits: N/A", 16, sf::Color(150, 150, 150));
  }
  y += 10.f;

  std::vector<ModuleDef> allInstalled;
  if (outfitterTab_ == 0) {
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
  }

  // Left: Player Modules
  float leftX = x;
  float rightX = rect.position.x + 400.f;
  float startY = y;

  if (outfitterTab_ == 0) {
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
        target.draw(t);
        y += 18.f;

        std::string statsLine = "    Vol: " + fmt(mdef.volumeOccupied, 1) +
                                "  Mass: " + fmt(mdef.mass, 1) +
                                "  Power: " + fmt(mdef.powerDraw, 1) + " GW";
        sf::Text st(*font, statsLine, 12);
        st.setFillColor(sf::Color(160, 160, 160));
        st.setPosition({leftX, y});
        target.draw(st);
        y += 18.f;

        if (sel) {
          for (const auto &attr : mdef.attributes) {
            sf::Text labelText(*font,
                               "    " + getAttributeName(attr.type) + ": ", 12);
            labelText.setFillColor(sf::Color(180, 180, 180));
            labelText.setPosition({leftX, y});
            target.draw(labelText);

            sf::Text starText(*font, getTierStars(attr.tier), 12);
            starText.setFillColor(sf::Color::Yellow);
            starText.setPosition({leftX + 130.f, y});
            target.draw(starText);

            y += 16.f;
          }
        }
      }
    }
  } else {
    dtext("Ammunition Inventory", 15, sf::Color::Yellow);
    if (registry.all_of<InstalledAmmo>(targetShip_)) {
      auto &ia = registry.get<InstalledAmmo>(targetShip_);
      if (ia.inventory.empty()) {
        dtext("No ammunition stored.", 14, sf::Color(150, 150, 150));
      } else {
        for (size_t i = 0; i < ia.inventory.size(); ++i) {
          auto &stack = ia.inventory[i];
          dtext("  " + std::to_string(stack.count) + "x " + stack.type.name, 14,
                sf::Color::White);
        }
      }
      y += 10.f;
      dtext(fmt(ia.usedVolume(), 1) + "/" + fmt(ia.totalCapacity(), 1) +
                " units used",
            12, sf::Color(160, 160, 160));
    } else {
      dtext("No Ammo Racks Installed.", 14, sf::Color::Red);
    }
  }

  // Right: Market Modules
  y = startY;
  x = rightX;
  std::vector<ModuleDef> shopModules;
  std::vector<AmmoDef> shopAmmo;

  uint32_t primaryFactionId = 0;
  if (registry.all_of<PlanetEconomy>(planetEntity_)) {
    auto &eco = registry.get<PlanetEconomy>(planetEntity_);
    float maxPop = -1.f;
    for (auto const &entry : eco.factionData) {
      uint32_t id = entry.first;
      auto const &fEco = entry.second;
      if (fEco.populationCount > maxPop) {
        maxPop = fEco.populationCount;
        primaryFactionId = id;
      }
    }
  }

  if (outfitterTab_ == 0) {
    dtext("Planet Market (Modules)", 15, sf::Color::Yellow);
    auto &eco = registry.get<PlanetEconomy>(planetEntity_);
    shopModules = eco.shopModules;
    for (int i = 0; i < (int)shopModules.size(); ++i) {
      bool sel = (outfitterMarketMode_ && i == selectedOutfitterIndex_);
      const auto &mdef = shopModules[i];
      ProductKey pk{ProductType::Module, (uint32_t)i, Tier::T1};

      float price = mdef.basePrice > 0.f ? mdef.basePrice : 500.f;

      std::string label =
          (sel ? "> " : "  ") + mdef.name + " $" + fmt(price, 0);
      sf::Text t(*font, label, 14);
      t.setFillColor(sel ? sf::Color::Cyan : sf::Color::White);
      t.setPosition({x, y});
      target.draw(t);
      y += 18.f;

      std::string sellerName = "Unknown";
      auto *sellerFaction =
          FactionManager::instance().getFactionPtr(mdef.originFactionId);
      if (sellerFaction)
        sellerName = sellerFaction->name;

      std::string statsLine = "    Seller: " + sellerName +
                              "  Vol: " + fmt(mdef.volumeOccupied, 1) +
                              "  Mass: " + fmt(mdef.mass, 1) +
                              "  Power: " + fmt(mdef.powerDraw, 1) + " GW";
      sf::Text st(*font, statsLine, 12);
      st.setFillColor(sf::Color(160, 160, 160));
      st.setPosition({x, y});
      target.draw(st);
      y += 18.f;

      if (sel) {
        for (const auto &attr : mdef.attributes) {
          sf::Text labelText(*font, "    " + getAttributeName(attr.type) + ": ",
                             12);
          labelText.setFillColor(sf::Color(180, 180, 180));
          labelText.setPosition({x, y});
          target.draw(labelText);

          sf::Text starText(*font, getTierStars(attr.tier), 12);
          starText.setFillColor(sf::Color::Yellow);
          starText.setPosition({x + 130.f, y});
          target.draw(starText);
          y += 16.f;
        }
      }
    }
  } else {
    dtext("Planet Market (Ammunition)", 15, sf::Color::Yellow);
    auto &eco = registry.get<PlanetEconomy>(planetEntity_);
    shopAmmo = eco.shopAmmo;
    for (int i = 0; i < (int)shopAmmo.size(); ++i) {
      bool sel = (outfitterMarketMode_ && i == selectedOutfitterIndex_);
      const auto &ammo = shopAmmo[i];

      std::string label =
          (sel ? "> " : "  ") + ammo.name + " $" + fmt(ammo.basePrice, 0);
      sf::Text t(*font, label, 14);
      t.setFillColor(sel ? sf::Color::Cyan : sf::Color::White);
      t.setPosition({x, y});
      target.draw(t);
      y += 18.f;

      std::string statsLine = "    Vol: " + fmt(ammo.volumePerRound, 1) +
                              "  Mass: " + fmt(ammo.massPerRound, 1);
      sf::Text st(*font, statsLine, 12);
      st.setFillColor(sf::Color(160, 160, 160));
      st.setPosition({x, y});
      target.draw(st);
      y += 18.f;
    }
  }

  x = rect.position.x + 20.f;
  y = rect.position.y + rect.size.y - 40.f;
  dtext("[Tab] Switch Panel   [Enter/B] Buy/Sell   [W/S] Navigate", 13,
        sf::Color(150, 150, 150));
}

} // namespace space
