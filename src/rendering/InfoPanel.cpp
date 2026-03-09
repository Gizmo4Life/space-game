#include "InfoPanel.h"
#include "UIUtils.h"
#include "game/FactionManager.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/NameComponent.h"

namespace space {

InfoPanel::InfoPanel(entt::entity planet) : planetEntity_(planet) {}

void InfoPanel::handleEvent(const sf::Event &, entt::registry &, b2WorldId) {
  // Info panel doesn't handle keyboard events for now
}

void InfoPanel::render(sf::RenderTarget &target, entt::registry &registry,
                       const sf::Font *font, sf::FloatRect rect) {
  drawPlanetInfo(target, registry, font, rect);
  drawFactionDNA(target, registry, font, rect);
}

void InfoPanel::drawPlanetInfo(sf::RenderTarget &target, entt::registry &r,
                               const sf::Font *f, sf::FloatRect rect) {
  if (!f || !r.valid(planetEntity_))
    return;

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 16.f;
  float lineH = 22.f;

  auto text = [&](const std::string &s, unsigned sz, sf::Color col) {
    sf::Text t(*f, s, sz);
    t.setFillColor(col);
    t.setPosition({x, y});
    target.draw(t);
    y += lineH;
  };

  std::string pname = r.all_of<NameComponent>(planetEntity_)
                          ? r.get<NameComponent>(planetEntity_).name
                          : "Unknown Planet";
  text(pname, 24, sf::Color(255, 220, 80));
  y += 4.f;

  if (r.all_of<CelestialBody>(planetEntity_)) {
    auto &cb = r.get<CelestialBody>(planetEntity_);
    text("Type: " + typeName(cb.type), 16, sf::Color(200, 200, 200));
  }

  if (r.all_of<Faction>(planetEntity_)) {
    auto &fac = r.get<Faction>(planetEntity_);
    uint32_t mid = fac.getMajorityFaction();
    auto &fd = FactionManager::instance().getFaction(mid);
    text("Faction: " + fd.name, 16, fd.color);
  }

  if (r.all_of<PlanetEconomy>(planetEntity_)) {
    auto &eco = r.get<PlanetEconomy>(planetEntity_);
    text("Population: " + fmt(eco.getTotalPopulation(), 1) + "k", 16,
         sf::Color(180, 255, 180));
    y += 8.f;

    text("── Market Prices ──", 15, sf::Color(140, 200, 255));
    std::vector<Resource> allRes = {
        Resource::Water,        Resource::Crops,
        Resource::Hydrocarbons, Resource::Metals,
        Resource::RareMetals,   Resource::Isotopes,
        Resource::Food,         Resource::Fuel,
        Resource::Weapons,      Resource::Electronics,
        Resource::Plastics,     Resource::ManufacturingGoods,
        Resource::Powercells};
    for (auto res : allRes) {
      ProductKey pk{ProductType::Resource, (uint32_t)res, Tier::T1};
      if (eco.currentPrices.count(pk)) {
        text("  " + getResourceName(res) + ": $" +
                 fmt(eco.currentPrices.at(pk)),
             14, sf::Color(210, 210, 210));
      }
    }
    y += 8.f;
    text("── Factions ──", 15, sf::Color(140, 200, 255));
    for (auto &[fId, fEco] : eco.factionData) {
      auto &fd = FactionManager::instance().getFaction(fId);
      text("  " + fd.name + ": " + fmt(fEco.populationCount, 1) + "k pop  $" +
               fmt(fEco.credits, 0),
           14, fd.color);
    }
  }
}

void InfoPanel::drawFactionDNA(sf::RenderTarget &target, entt::registry &r,
                               const sf::Font *f, sf::FloatRect rect) {
  if (!f || !r.valid(planetEntity_) || !r.all_of<Faction>(planetEntity_))
    return;

  auto &facComp = r.get<Faction>(planetEntity_);
  uint32_t fId = facComp.getMajorityFaction();
  auto &fd = FactionManager::instance().getFaction(fId);

  float x = rect.position.x + 20.f;
  float y = rect.position.y + rect.size.y - 170.f;
  float lineH = 20.f;

  auto text = [&](const std::string &s, unsigned sz, sf::Color col) {
    sf::Text t(*f, s, sz);
    t.setFillColor(col);
    t.setPosition({x, y});
    target.draw(t);
    y += lineH;
  };

  text("── Faction Profile: " + fd.name + " ──", 15, sf::Color(140, 200, 255));
  y += 5.f;

  auto drawAxis = [&](const std::string &label, float val) {
    sf::Text lbl(*f, label, 13);
    lbl.setFillColor(sf::Color(200, 200, 200));
    lbl.setPosition({x, y});
    target.draw(lbl);

    float barW = 120.f;
    float barH = 6.f;
    sf::RectangleShape bg({barW, barH});
    bg.setPosition({x + 110.f, y + 6.f});
    bg.setFillColor(sf::Color(40, 40, 60));
    target.draw(bg);

    sf::RectangleShape bar({barW * val, barH});
    bar.setPosition({x + 110.f, y + 6.f});
    bar.setFillColor(fd.color);
    target.draw(bar);
    y += lineH;
  };

  drawAxis("Aggression", fd.dna.aggression);
  drawAxis("Industrialism", fd.dna.industrialism);
  drawAxis("Commercialism", fd.dna.commercialism);
  drawAxis("Cooperation", fd.dna.cooperation);

  y += 5.f;
  float rel = FactionManager::instance().getRelationship(0, fId);
  std::string relStr = "Neutral";
  sf::Color relCol = sf::Color(200, 200, 200);
  if (rel > 0.4f) {
    relStr = "Friendly";
    relCol = sf::Color(100, 255, 100);
  } else if (rel < -0.4f) {
    relStr = "Hostile";
    relCol = sf::Color(255, 100, 100);
  }
  text("Relationship: " + relStr + " (" + fmt(rel, 2) + ")", 14, relCol);
}

} // namespace space
