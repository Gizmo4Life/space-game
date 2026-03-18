#include "OutfitterPanel.h"
#include "UIUtils.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

namespace space {

OutfitterPanel::OutfitterPanel(entt::entity planet, entt::entity player)
    : planetEntity_(planet), playerEntity_(player) {}

void OutfitterPanel::handleEvent(const sf::Event &event, const UIContext &ctx,
                                 b2WorldId worldId) {
  auto &registry = ctx.registry;
  if (const auto *kp = event.getIf<sf::Event::KeyPressed>()) {
    if (kp->code == sf::Keyboard::Key::Tab) {
      outfitterMarketMode_ = !outfitterMarketMode_;
      selectedOutfitterIndex_ = 0;
      detailScrollY_ = 0.f;
    }
    if (kp->code == sf::Keyboard::Key::Z) {
      outfitterTab_ = 1 - outfitterTab_; // Toggle Modules/Ammo
      selectedOutfitterIndex_ = 0;
      detailScrollY_ = 0.f;
    }

    // Ship Switching
    if (kp->code == sf::Keyboard::Key::Left ||
        kp->code == sf::Keyboard::Key::A) {
      // Find previous ship in fleet
      std::vector<entt::entity> fleet;
      auto playerView = registry.view<PlayerComponent, HullDef, NameComponent>();
      for (auto e : playerView)
        if (playerView.get<PlayerComponent>(e).isFlagship)
          fleet.push_back(e);
      auto npcView = registry.view<NPCComponent, HullDef, NameComponent>();
      for (auto e : npcView)
        if (npcView.get<NPCComponent>(e).isPlayerFleet)
          fleet.push_back(e);

      auto it = std::find(fleet.begin(), fleet.end(), targetShip_);
      if (it != fleet.end() && it != fleet.begin()) {
        targetShip_ = *(--it);
      } else if (!fleet.empty()) {
        targetShip_ = fleet.back();
      }
      selectedOutfitterIndex_ = 0;
      detailScrollY_ = 0.f;
    }

    if (kp->code == sf::Keyboard::Key::Right ||
        kp->code == sf::Keyboard::Key::D) {
      std::vector<entt::entity> fleet;
      auto playerView = registry.view<PlayerComponent, HullDef, NameComponent>();
      for (auto e : playerView)
        if (playerView.get<PlayerComponent>(e).isFlagship)
          fleet.push_back(e);
      auto npcView = registry.view<NPCComponent, HullDef, NameComponent>();
      for (auto e : npcView)
        if (npcView.get<NPCComponent>(e).isPlayerFleet)
          fleet.push_back(e);

      auto it = std::find(fleet.begin(), fleet.end(), targetShip_);
      if (it != fleet.end() && std::next(it) != fleet.end()) {
        targetShip_ = *(++it);
      } else if (!fleet.empty()) {
        targetShip_ = fleet.front();
      }
      selectedOutfitterIndex_ = 0;
      detailScrollY_ = 0.f;
    }

    // List Navigation
    if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W) {
      if (selectedOutfitterIndex_ > 0) {
        selectedOutfitterIndex_--;
        detailScrollY_ = 0.f;
      }
    }

    // Get max index for currently active side
    int maxIdx = 0;
    if (outfitterMarketMode_) {
      if (outfitterTab_ == 0) {
        if (registry.all_of<PlanetEconomy>(planetEntity_))
          maxIdx = (int)registry.get<PlanetEconomy>(planetEntity_)
                       .shopModules.size();
      } else {
        if (registry.all_of<PlanetEconomy>(planetEntity_))
          maxIdx =
              (int)registry.get<PlanetEconomy>(planetEntity_).shopAmmo.size();
      }
    } else {
      if (outfitterTab_ == 0) {
        // Collect installed
        if (registry.valid(targetShip_)) {
          auto collect = [&](const std::vector<ModuleDef> &modules) {
            for (const auto &m : modules)
              if (!m.name.empty() && m.name != "Empty")
                maxIdx++;
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
      } else {
        if (registry.all_of<InstalledAmmo>(targetShip_))
          maxIdx =
              (int)registry.get<InstalledAmmo>(targetShip_).inventory.size();
      }
    }

    if (kp->code == sf::Keyboard::Key::Down ||
        kp->code == sf::Keyboard::Key::S) {
      if (selectedOutfitterIndex_ < maxIdx - 1) {
        selectedOutfitterIndex_++;
        detailScrollY_ = 0.f;
      }
    }

    // Detail scrolling [ and ]
    if (kp->code == sf::Keyboard::Key::LBracket) {
      detailScrollY_ = std::max(0.f, detailScrollY_ - 40.f);
    }
    if (kp->code == sf::Keyboard::Key::RBracket) {
      detailScrollY_ += 40.f;
    }

    // Buy / Sell / Refit
    if (kp->code == sf::Keyboard::Key::Enter ||
        kp->code == sf::Keyboard::Key::B) {
      if (outfitterMarketMode_) {
        // Buy from market to targetShip
        if (outfitterTab_ == 0 &&
            registry.all_of<PlanetEconomy>(planetEntity_)) {
          auto &eco = registry.get<PlanetEconomy>(planetEntity_);
          if (selectedOutfitterIndex_ < (int)eco.shopModules.size()) {
            ProductKey pk{ProductType::Module,
                          (uint32_t)selectedOutfitterIndex_, Tier::T1};
            ShipOutfitter::instance().refitModule(registry, targetShip_,
                                                  planetEntity_, pk, 0);
          }
        }
      } else {
        // Sell from ship to market (REQ-13)
        if (outfitterTab_ == 0 && registry.valid(targetShip_)) {
          struct Entry {
            ModuleCategory cat;
            int idx;
          };
          std::vector<Entry> entries;
          auto collect = [&](ModuleCategory c,
                             const std::vector<ModuleDef> &modules) {
            for (int i = 0; i < (int)modules.size(); ++i)
              if (!modules[i].name.empty() && modules[i].name != "Empty")
                entries.push_back({c, i});
          };
          if (registry.all_of<InstalledEngines>(targetShip_))
            collect(ModuleCategory::Engine,
                    registry.get<InstalledEngines>(targetShip_).modules);
          if (registry.all_of<InstalledWeapons>(targetShip_))
            collect(ModuleCategory::Weapon,
                    registry.get<InstalledWeapons>(targetShip_).modules);
          if (registry.all_of<InstalledShields>(targetShip_))
            collect(ModuleCategory::Shield,
                    registry.get<InstalledShields>(targetShip_).modules);
          if (registry.all_of<InstalledCargo>(targetShip_))
            collect(ModuleCategory::Utility,
                    registry.get<InstalledCargo>(targetShip_).modules);
          if (registry.all_of<InstalledPower>(targetShip_))
            collect(ModuleCategory::Reactor,
                    registry.get<InstalledPower>(targetShip_).modules);
          if (registry.all_of<InstalledReactionWheels>(targetShip_))
            collect(ModuleCategory::ReactionWheel,
                    registry.get<InstalledReactionWheels>(targetShip_).modules);

          if (selectedOutfitterIndex_ < (int)entries.size()) {
            auto &e = entries[selectedOutfitterIndex_];
            ShipOutfitter::instance().sellModule(registry, targetShip_,
                                                  planetEntity_, e.cat, e.idx);
          }
        }
      }
    }
  }
}

void OutfitterPanel::render(sf::RenderTarget &target, const UIContext &ctx,
                            const sf::Font *font, sf::FloatRect rect) {
  if (!font || !ctx.registry.valid(ctx.player))
    return;

  auto &registry = ctx.registry;
  auto playerEntity = ctx.player;

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 20.f;

  auto dtext = [&](float coreX, float &coreY, const std::string &s, unsigned sz,
                   sf::Color c) {
    sf::Text t(*font, s, sz);
    t.setFillColor(c);
    t.setPosition({coreX, coreY});
    target.draw(t);
    coreY += sz + 6.f;
  };

  if (!registry.valid(targetShip_) || !registry.all_of<HullDef, NameComponent>(targetShip_)) {
    targetShip_ = playerEntity;
  }

  if (!registry.valid(targetShip_) || !registry.all_of<HullDef, NameComponent>(targetShip_)) {
    dtext(x, y, "Vessel data is temporarily unavailable", 20, sf::Color::Yellow);
    return;
  }

  dtext(x, y, "── Vessel Outfitter ──", 18, sf::Color(140, 200, 255));

  // Top Pane: Ship Selection & Status
  if (registry.all_of<HullDef>(targetShip_)) {
    const auto &hdef = registry.get<HullDef>(targetShip_);
    dtext(x, y, hdef.name + " (" + hdef.className + ")", 20, sf::Color::Cyan);
    dtext(x, y, "A/D to cycle Fleet vessels", 12, sf::Color(150, 150, 150));

    // Stats summary
    float usedVol = 0.f;
    float powerTotal = 0.f;
    auto collect = [&](const std::vector<ModuleDef> &modules) {
      for (const auto &m : modules) {
        if (m.name.empty() || m.name == "Empty")
          continue;
        usedVol += m.volumeOccupied;
        powerTotal -= m.powerDraw;
      }
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

    dtext(x, y,
          "Vol: " + fmt(usedVol, 1) + "/" + fmt(hdef.internalVolume, 0) +
              "  Power: " + fmt(powerTotal, 1) + " GW",
          14, sf::Color::White);
  } else {
    dtext(x, y, "No Ship Target Selected", 20, sf::Color::Red);
  }

  CreditsComponent *pCredits =
      registry.try_get<CreditsComponent>(playerEntity);
  dtext(x, y, "Credits: $" + (pCredits ? fmt(pCredits->amount, 0) : "0"), 16,
        sf::Color(100, 255, 100));
  y += 10.f;

  float listStartY = y;
  float colWidth = 320.f;
  int maxVisible = 18;

  // Sync scroll offsets
  if (!outfitterMarketMode_) {
    if (selectedOutfitterIndex_ < installedScrollOffset_)
      installedScrollOffset_ = selectedOutfitterIndex_;
    else if (selectedOutfitterIndex_ >= installedScrollOffset_ + maxVisible)
      installedScrollOffset_ = selectedOutfitterIndex_ - maxVisible + 1;
  } else {
    if (selectedOutfitterIndex_ < marketScrollOffset_)
      marketScrollOffset_ = selectedOutfitterIndex_;
    else if (selectedOutfitterIndex_ >= marketScrollOffset_ + maxVisible)
      marketScrollOffset_ = selectedOutfitterIndex_ - maxVisible + 1;
  }

  // --- Left Column: Installed ---
  float lx = x;
  float ly = listStartY;
  dtext(lx, ly, "── Installed ──", 15, sf::Color::Yellow);

  std::vector<ModuleDef> allInstalled;
  if (outfitterTab_ == 0 && registry.valid(targetShip_)) {
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

  int installEnd =
      std::min((int)allInstalled.size(), installedScrollOffset_ + maxVisible);
  for (int i = installedScrollOffset_; i < installEnd; ++i) {
    bool sel = (!outfitterMarketMode_ && i == selectedOutfitterIndex_);
    dtext(lx, ly, (sel ? "> " : "  ") + allInstalled[i].name, 14,
          sel ? sf::Color::Cyan : sf::Color::White);
  }

  // --- Middle Column: Market ---
  float mx = x + colWidth;
  float my = listStartY;
  dtext(mx, my, "── Market ──", 15, sf::Color::Yellow);

  std::vector<ModuleDef> shopModules;
  if (registry.all_of<PlanetEconomy>(planetEntity_)) {
    shopModules = registry.get<PlanetEconomy>(planetEntity_).shopModules;
  }

  int marketEnd =
      std::min((int)shopModules.size(), marketScrollOffset_ + maxVisible);
  for (int i = marketScrollOffset_; i < marketEnd; ++i) {
    bool sel = (outfitterMarketMode_ && i == selectedOutfitterIndex_);
    dtext(mx, my,
          (sel ? "> " : "  ") + shopModules[i].name + " $" +
              fmt(shopModules[i].basePrice, 0),
          14, sel ? sf::Color::Cyan : sf::Color::White);
  }

  // --- Right Column: Detail Pane ---
  float dx = x + colWidth * 2.f;
  float dy_start = listStartY;
  float dy = dy_start - detailScrollY_;

  ModuleDef selMod;
  bool hasSelection = false;
  if (outfitterMarketMode_ &&
      selectedOutfitterIndex_ < (int)shopModules.size()) {
    selMod = shopModules[selectedOutfitterIndex_];
    hasSelection = true;
  } else if (!outfitterMarketMode_ &&
              selectedOutfitterIndex_ < (int)allInstalled.size()) {
    selMod = allInstalled[selectedOutfitterIndex_];
    hasSelection = true;
  }

  if (hasSelection) {
    auto dDetail = [&](const std::string &s, unsigned sz, sf::Color c) {
      if (dy >= dy_start && dy < rect.position.y + rect.size.y - 40.f) {
        sf::Text t(*font, s, sz);
        t.setFillColor(c);
        t.setPosition({dx, dy});
        target.draw(t);
      }
      dy += sz + 6.f;
    };

    dDetail(selMod.name, 18, sf::Color::Yellow);
    dDetail("Category: " + moduleCategoryName(selMod.category), 12,
            sf::Color(180, 180, 180));
    dDetail("Vol: " + fmt(selMod.volumeOccupied, 1) +
                "  Mass: " + fmt(selMod.mass, 1),
            14, sf::Color::White);
    dDetail("Power: " + fmt(selMod.powerDraw, 1) + " GW", 14,
            selMod.powerDraw > 0 ? sf::Color::Yellow
                                 : sf::Color(100, 255, 100));
    dy += 10.f;
    dDetail("── Attributes ──", 14, sf::Color(140, 200, 255));
    for (const auto &attr : selMod.attributes) {
      dDetail("• " + getAttributeName(attr.type) + " " +
                   getTierStars(attr.tier),
              12, sf::Color::White);
    }
  }

  // Visual Dividers (REQ-09)
  sf::RectangleShape div1({1.5f, rect.size.y - 120.f});
  div1.setFillColor(sf::Color(80, 80, 100));
  div1.setPosition({rect.position.x + colWidth, listStartY});
  target.draw(div1);

  sf::RectangleShape div2({1.5f, rect.size.y - 120.f});
  div2.setFillColor(sf::Color(80, 80, 100));
  div2.setPosition({rect.position.x + colWidth * 2.f, listStartY});
  target.draw(div2);

  // Footer
  float helpY = rect.position.y + rect.size.y - 35.f;
  sf::Text help(*font,
                "[Tab] Swap Column  [W/S] Nav  [Enter/B] Buy/Sell  [[]/[]] "
                "Scroll Details",
                13);
  help.setFillColor(sf::Color(150, 150, 150));
  help.setPosition({x, helpY});
  target.draw(help);
}

} // namespace space
