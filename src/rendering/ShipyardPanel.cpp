#include "ShipyardPanel.h"
#include "ShipRenderer.h"
#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/Faction.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/ShipModule.h"
#include "game/components/ShipFitness.h"
#include "game/components/ShipStats.h"
#include "game/components/InstalledModules.h"
#include "rendering/UIUtils.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <algorithm>
#include <entt/entt.hpp>
#include <opentelemetry/trace/provider.h>
#include <vector>

namespace space {

ShipyardPanel::ShipyardPanel(entt::entity planet, entt::entity player)
    : planetEntity_(planet), playerEntity_(player) {}

void ShipyardPanel::handleEvent(const sf::Event &event, const UIContext &ctx,
                                b2WorldId worldId) {
  auto &registry = ctx.registry;
  auto playerEntity = ctx.player;

  if (const auto *kp = event.getIf<sf::Event::KeyPressed>()) {
    if (kp->code == sf::Keyboard::Key::Tab) {
      mode_ =
          (mode_ == ShipyardMode::Buy) ? ShipyardMode::Sell : ShipyardMode::Buy;
      selectedBidIndex_ = 0;
      scrollOffset_ = 0;
      detailScrollY_ = 0.f;

      if (mode_ == ShipyardMode::Sell) {
        getFleetEntities(registry, playerEntity, fleetEntities_);
      }
    }

    int maxIdx = (mode_ == ShipyardMode::Buy) ? (int)currentBids_.size()
                                              : (int)fleetEntities_.size();

    if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W) {
      if (selectedBidIndex_ > 0) {
        selectedBidIndex_--;
        detailScrollY_ = 0.f;
      }
    }
    if (kp->code == sf::Keyboard::Key::Down ||
        kp->code == sf::Keyboard::Key::S) {
      if (selectedBidIndex_ < maxIdx - 1) {
        selectedBidIndex_++;
        detailScrollY_ = 0.f;
      }
    }

    // Manual detail scrolling (requested [ and ])
    if (kp->code == sf::Keyboard::Key::LBracket) {
      detailScrollY_ = std::max(0.f, detailScrollY_ - 40.f);
    }
    if (kp->code == sf::Keyboard::Key::RBracket) {
      detailScrollY_ += 40.f;
    }

    // Purchase/Sell Logic
    bool buyToFleet = (kp->code == sf::Keyboard::Key::B);
    bool buyAsFlagship = (kp->code == sf::Keyboard::Key::F);
    bool sellShip = (kp->code == sf::Keyboard::Key::X);

    if (mode_ == ShipyardMode::Buy && (buyToFleet || buyAsFlagship) &&
        !currentBids_.empty()) {
      auto span =
          Telemetry::instance().tracer()->StartSpan("game.ui.ship.purchase");
      const auto &bid = currentBids_[selectedBidIndex_];

      auto &credits = registry.get<CreditsComponent>(playerEntity);
      bool canAfford = credits.amount >= bid.price;

      if (canAfford) {
        if (EconomyManager::instance().buyShip(registry, planetEntity_,
                                               playerEntity, bid, worldId,
                                               buyToFleet, buyAsFlagship)) {

          // Re-sync after flagship change
          playerEntity = findFlagship(registry);

          currentBids_ =
              EconomyManager::instance().getHullBids(registry, planetEntity_);
          span->SetAttribute("purchase.success", true);
        } else {
          span->SetAttribute("purchase.denied", "buyShip returned false");
        }
      } else {
        span->SetAttribute("purchase.denied", "insufficient credits");
      }
      span->End();
    } else if (mode_ == ShipyardMode::Sell &&
               (sellShip || kp->code == sf::Keyboard::Key::E) &&
               !fleetEntities_.empty()) {
      entt::entity toSell = fleetEntities_[selectedBidIndex_];
      bool isTransfer = (kp->code == sf::Keyboard::Key::E);

      auto span = Telemetry::instance().tracer()->StartSpan(
          isTransfer ? "game.ui.ship.transfer" : "game.ui.ship.sell");

      bool success = false;
      if (isTransfer) {
        uint32_t fId = 0;
        if (registry.all_of<Faction>(playerEntity)) {
          fId = registry.get<Faction>(playerEntity).getMajorityFaction();
        }
        success =
            EconomyManager::instance().transferShipToFaction(registry, toSell, fId);
      } else {
        success = EconomyManager::instance().sellShip(registry, planetEntity_,
                                                      playerEntity, toSell);
      }

      if (success) {
        // Re-sync playerEntity if flagship was sold/transferred
        playerEntity = findFlagship(registry);
        // Refresh list
        getFleetEntities(registry, playerEntity, fleetEntities_);
        selectedBidIndex_ =
            std::min(selectedBidIndex_, (int)fleetEntities_.size() - 1);
        span->SetAttribute(isTransfer ? "transfer.success" : "sell.success", true);
      }
      span->End();
    }
  }
}

void ShipyardPanel::render(sf::RenderTarget &target, const UIContext &ctx,
                           const sf::Font *font, sf::FloatRect rect) {
  if (!font)
    return;

  auto &registry = ctx.registry;
  auto playerEntity = ctx.player;

  if (mode_ == ShipyardMode::Buy) {
    currentBids_ =
        EconomyManager::instance().getHullBids(registry, planetEntity_);
    if (selectedBidIndex_ >= (int)currentBids_.size())
      selectedBidIndex_ = std::max(0, (int)currentBids_.size() - 1);
  } else {
    if (selectedBidIndex_ >= (int)fleetEntities_.size())
      selectedBidIndex_ = std::max(0, (int)fleetEntities_.size() - 1);
  }

  // Automatic scrolling for the list
  int maxVisible = 18;
  if (selectedBidIndex_ < scrollOffset_) {
    scrollOffset_ = selectedBidIndex_;
  } else if (selectedBidIndex_ >= scrollOffset_ + maxVisible) {
    scrollOffset_ = selectedBidIndex_ - maxVisible + 1;
  }

  float listWidth = 340.f;
  float x = rect.position.x + 20.f;
  float y = rect.position.y + 20.f;

  sf::Text title(*font,
                 (mode_ == ShipyardMode::Buy ? "── Available Vessels ──"
                                             : "── Your Fleet (Sell) ──"),
                 18);
  title.setFillColor(sf::Color(140, 200, 255));
  title.setPosition({x, y});
  target.draw(title);
  y += 35.f;

  if (mode_ == ShipyardMode::Buy) {
    if (currentBids_.empty()) {
      sf::Text none(*font, "No vessels for sale here.", 16);
      none.setFillColor(sf::Color(180, 180, 180));
      none.setPosition({x, y});
      target.draw(none);
    } else {
      int endIdx =
          std::min((int)currentBids_.size(), scrollOffset_ + maxVisible);
      for (int i = scrollOffset_; i < endIdx; ++i) {
        const auto &bid = currentBids_[i];
        bool sel = (i == selectedBidIndex_);
        sf::Color col = sel ? sf::Color::Cyan : sf::Color::White;
        const auto &faction =
            FactionManager::instance().getFaction(bid.factionId);
        std::string label = (sel ? "> " : "  ") + bid.blueprint.hull.className +
                            " (" + bid.blueprint.role + ") [" + faction.name +
                            "]";
        sf::Text t(*font, label, 15);
        t.setFillColor(col);
        t.setPosition({x, y});
        target.draw(t);
        y += 20.f;
      }
    }
  } else {
    if (fleetEntities_.empty()) {
      sf::Text none(*font, "No ships in your fleet.", 16);
      none.setFillColor(sf::Color(180, 180, 180));
      none.setPosition({x, y});
      target.draw(none);
    } else {
      int endIdx =
          std::min((int)fleetEntities_.size(), scrollOffset_ + maxVisible);
      for (int i = scrollOffset_; i < endIdx; ++i) {
        auto entity = fleetEntities_[i];
        if (!registry.valid(entity))
          continue;
        bool sel = (i == selectedBidIndex_);
        sf::Color col = sel ? sf::Color::Cyan : sf::Color::White;
        auto *nc = registry.try_get<NameComponent>(entity);
        std::string name = nc ? nc->name : "Unknown Ship";
        std::string label = (sel ? "> " : "  ") + name;
        sf::Text t(*font, label, 15);
        t.setFillColor(col);
        t.setPosition({x, y});
        target.draw(t);
        y += 20.f;
      }
    }
  }

  // Divider
  sf::RectangleShape vDiv({2.f, rect.size.y - 40.f});
  vDiv.setPosition({rect.position.x + listWidth, rect.position.y + 20.f});
  vDiv.setFillColor(sf::Color(80, 80, 100));
  target.draw(vDiv);

  // Detail Pane (Right Side)
  float dx = rect.position.x + listWidth + 20.f;
  float dy_start = rect.position.y + 20.f;
  float dy = dy_start - detailScrollY_;

  auto dtext = [&](float coreX, float &coreY, const std::string &s, unsigned sz,
                   sf::Color c) {
    if (coreY >= dy_start && coreY < rect.position.y + rect.size.y - 40.f) {
      sf::Text t(*font, s, sz);
      t.setFillColor(c);
      t.setPosition({coreX, coreY});
      target.draw(t);
    }
    coreY += sz + 6.f;
  };

  ShipBlueprint bp;
  uint32_t bidFactionId = 0;
  float displayPrice = 0.f;

  if (mode_ == ShipyardMode::Buy && !currentBids_.empty()) {
    const auto &bid = currentBids_[selectedBidIndex_];
    bp = bid.blueprint;
    bidFactionId = bid.factionId;
    displayPrice = bid.price;
  } else if (mode_ == ShipyardMode::Sell && !fleetEntities_.empty()) {
    entt::entity e = fleetEntities_[selectedBidIndex_];
    if (!registry.valid(e))
      return;
    auto *hdef = registry.try_get<HullDef>(e);
    if (!hdef)
      return;
    // Create a pseudo-blueprint for display
    bp.hull = *hdef;
    bp.role = "Owned";
    auto valResult = ShipOutfitter::instance().calculateDetailedShipValue(registry, e);
    displayPrice = valResult.total * 0.8f;
    if (registry.all_of<Faction>(e))
      bidFactionId = registry.get<Faction>(e).getMajorityFaction();
  } else {
    return; // Nothing to show
  }

  const auto &faction = FactionManager::instance().getFaction(bidFactionId);
  dtext(dx, dy, bp.hull.className + " Details", 22, sf::Color::Yellow);
  dtext(dx, dy, "Tier: " + tierName(bp.hull.sizeTier), 14,
        sf::Color(180, 180, 180));

  bool isTransfer = false;
  if (registry.all_of<Faction>(playerEntity)) {
    if (registry.get<Faction>(playerEntity).getMajorityFaction() ==
        bidFactionId) {
      isTransfer = true;
    }
  }

  std::string priceLabel =
      (mode_ == ShipyardMode::Buy
           ? (isTransfer ? "Transfer: " : "Buy Price: $")
           : "Sell Value: $");
  std::string priceVal =
      (isTransfer && mode_ == ShipyardMode::Buy ? "FREE (Faction Collection)"
                                                : fmt(displayPrice, 0));

  dtext(dx, dy, priceLabel + priceVal, 18, sf::Color(100, 255, 100));
  dy += 10.f;
  dtext(dx, dy, "── Valuation Breakdown ──", 14, sf::Color(140, 200, 255));
  dy += 20.f;
  
  if (mode_ == ShipyardMode::Sell && !fleetEntities_.empty()) {
      entt::entity e = fleetEntities_[selectedBidIndex_];
      auto val = ShipOutfitter::instance().calculateDetailedShipValue(registry, e);
      dtext(dx, dy, "  Hull: $" + fmt(val.hullValue * 0.8f, 0), 12, sf::Color::White); dy += 16.f;
      dtext(dx, dy, "  Modules: $" + fmt(val.moduleValue * 0.8f, 0), 12, sf::Color::White); dy += 16.f;
      dtext(dx, dy, "  Cargo: $" + fmt(val.cargoValue * 0.8f, 0), 12, sf::Color::White); dy += 16.f;
      dtext(dx, dy, "  Ammo: $" + fmt(val.ammoValue * 0.8f, 0), 12, sf::Color::White); dy += 16.f;
  } else if (mode_ == ShipyardMode::Buy) {
      // For buying, we show the base values (without trade-in discount)
      dtext(dx, dy, "  Base Hull: $" + fmt(bp.hull.baseMass * 100.0f, 0), 12, sf::Color::White); dy += 16.f;
      float mVal = 0;
      for (const auto& m : bp.modules) if (m.name != "Empty") mVal += m.basePrice;
      dtext(dx, dy, "  Modules: $" + fmt(mVal, 0), 12, sf::Color::White); dy += 16.f;
      float aVal = 0;
      for (const auto& s : bp.startingAmmo) aVal += s.count * (s.type.basePrice > 0 ? s.type.basePrice : 10.0f);
      dtext(dx, dy, "  Included Ammo: $" + fmt(aVal, 0), 12, sf::Color::White); dy += 16.f;
  }

  // Ship Preview
  sf::Vector2f previewPos = {dx + 350.f, rect.position.y + 120.f};
  ShipRenderParams sparams;
  sparams.mode = RenderMode::Schematic;
  sparams.color = faction.color;
  sparams.scale = 1.0f;
  sparams.viewScale = 6.0f;
  ShipRenderer::drawShip(target, bp.hull, previewPos, sparams);

  // Technical Stats
  float usedVol = 0.f;
  float totalVol = bp.hull.internalVolume;
  float powerDraw = 0.f;
  float totalMass = 0.f;
  float thrust = 0.f;
  float shieldCap = 0.f;
  float outputPower = 0.f;

  if (mode_ == ShipyardMode::Buy) {
    auto bstats = bp.calculateStats();
    usedVol = bstats.totalVolume;
    powerDraw = bstats.totalPowerDraw;
    totalMass = bstats.totalMass;
    thrust = bstats.totalThrust;
    shieldCap = bstats.totalShield;
    outputPower = bstats.totalOutput;
  } else {
    entt::entity e = fleetEntities_[selectedBidIndex_];
    if (auto *s = registry.try_get<ShipStats>(e)) {
      usedVol = s->internalVolumeOccupied;
      powerDraw = s->restingPowerDraw;
      totalMass = s->wetMass;
      if (registry.all_of<InstalledEngines>(e)) thrust = registry.get<InstalledEngines>(e).totalThrust;
      if (registry.all_of<InstalledShields>(e)) shieldCap = registry.get<InstalledShields>(e).maxShield;
      if (registry.all_of<InstalledPower>(e)) outputPower = registry.get<InstalledPower>(e).output;
    }
  }

  dtext(dx, dy, "Volume: " + fmt(usedVol, 1) + " / " + fmt(totalVol, 0), 14,
        (usedVol > totalVol) ? sf::Color::Red : sf::Color::Cyan);
  dtext(dx, dy, "Mass: " + fmt(totalMass, 1) + " t", 14, sf::Color::White);
  dtext(dx, dy, "Thrust: " + fmt(thrust, 0) + " N", 14, sf::Color(255, 200, 100));
  dtext(dx, dy, "Shield: " + fmt(shieldCap, 0) + " Units", 14, sf::Color(150, 200, 255));
  dtext(dx, dy, "Reactor: " + fmt(outputPower, 0) + " GW", 14, sf::Color(100, 255, 100));
  dtext(dx, dy, "Power Draw (Net): " + fmt(powerDraw, 1) + " GW", 14,
        (powerDraw > 1.f) ? sf::Color::Yellow : sf::Color(100, 255, 100));

  // Fitness Score
  float fitness = 0.0f;
  auto it = faction.dna.tierDNA.find(bp.hull.sizeTier);
  TierDNA tdna = (it != faction.dna.tierDNA.end()) ? it->second : TierDNA();

  if (bp.role == "Combat")
    fitness = ShipFitness::calculateCombatFitness(bp, tdna);
  else if (bp.role == "Cargo")
    fitness = ShipFitness::calculateTradeFitness(bp, tdna);
  else if (bp.role == "Transport")
    fitness = ShipFitness::calculateTransportFitness(bp, tdna);
  else
    fitness =
        ShipFitness::calculateGeneralFitness(bp, faction.dna, bp.hull.sizeTier);

  sf::Color fitCol = sf::Color::Green;
  if (fitness <= 0.0f)
    fitCol = sf::Color::Red;
  else if (fitness < 0.4f)
    fitCol = sf::Color(255, 165, 0); // Orange

  dtext(dx, dy, "Fitness: " + fmt(fitness * 100.0f, 1) + "%", 16, fitCol);

  // Survivors
  dy += 10.f;
  dtext(dx, dy, "── Survival TTE (Equipped) ──", 16, sf::Color(140, 200, 255));

  float foodTTE = 5.0f;
  float fuelTTE = 5.0f;
  float isotopesTTE = 5.0f;
  float ammoTTE = 5.0f;
  bool showAmmo = false;

  if (mode_ == ShipyardMode::Sell && !fleetEntities_.empty()) {
    entt::entity e = fleetEntities_[selectedBidIndex_];
    if (auto *s = registry.try_get<ShipStats>(e)) {
      foodTTE = s->foodTTE;
      fuelTTE = s->fuelTTE;
      isotopesTTE = s->isotopesTTE;
      ammoTTE = s->ammoTTE;
      showAmmo = (s->ammoCapacity > 0);
    }
  } else {
    // For Bids: Bids come fully stocked (Standard TTE is 5 days)
    // We only show ammo if kinetic modules are present
    for (const auto &m : bp.modules) {
      if (m.category == ModuleCategory::Weapon &&
          m.weaponType != WeaponType::Energy && !m.name.empty() &&
          m.name != "Empty") {
        showAmmo = true;
        break;
      }
    }
  }

  auto dtte = [&](const std::string &label, float val) {
    std::string v =
        (val < 0 || val > 90.0f) ? "Infinite" : fmt(val, 1) + " days";
    dtext(dx + 10.f, dy, label + ": " + v, 14, sf::Color::White);
  };

  dtte("Food", foodTTE);
  dtte("Fuel", fuelTTE);
  dtte("Isotopes", isotopesTTE);
  if (showAmmo) {
    dtte("Ammo", ammoTTE);
  } else if (mode_ == ShipyardMode::Buy) {
    dtext(dx + 10.f, dy, "Ammo: Infinite (Energy Only)", 14,
          sf::Color(180, 180, 180));
  }

  dy += 15.f;
  dtext(dx, dy, "── Ammunition Loadout ──", 16, sf::Color(140, 200, 255));
  if (bp.startingAmmo.empty()) {
      dtext(dx, dy, "• None", 14, sf::Color(180, 180, 180));
  } else {
      for (const auto &stack : bp.startingAmmo) {
          dtext(dx, dy, "• " + stack.type.name + ": " + std::to_string(stack.count), 14, sf::Color::White);
      }
  }

  dy += 15.f;
  dtext(dx, dy, "── Installed Modules ──", 16, sf::Color(140, 200, 255));

  for (const auto &m : bp.modules) {
    if (m.name.empty() || m.name == "Empty")
      continue;
    dtext(dx, dy, "• " + m.name, 14, sf::Color::White);
    for (const auto &attr : m.attributes) {
      dtext(dx + 20.f, dy,
            getAttributeName(attr.type) + " " + getTierStars(attr.tier), 11,
            sf::Color(180, 180, 180));
    }
    dy += 5.f;
  }

  float helpY = rect.position.y + rect.size.y - 35.f;
  std::string instructions =
      (mode_ == ShipyardMode::Buy)
          ? "[B] Buy/Transfer to Fleet  [F] Buy/Transfer as Flagship  [Tab] Sell Mode  [W/S] Nav "
            " [[ / ]] Scroll Details"
          : "[X] SELL SHIP  [E] Transfer to Faction  [Tab] Buy Mode  [W/S] Nav  [[ / ]] Scroll Details";
  sf::Text help(*font, instructions, 13);
  help.setFillColor(sf::Color(150, 150, 150));
  help.setPosition({rect.position.x + 20.f, helpY});
  target.draw(help);
}

} // namespace space
