#include "FactionManager.h"
#include "engine/telemetry/Telemetry.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <opentelemetry/trace/provider.h>

namespace space {

void FactionManager::init() {
  factions.clear();
  relationships.clear();

  // Faction 0 is always "Civilian" / Neutral
  FactionData civ;
  civ.id = 0;
  civ.name = "Civilian";
  civ.color = sf::Color(150, 150, 150);
  civ.militaryWeight = 0.05f;
  civ.freightWeight = 0.45f;
  civ.passengerWeight = 0.5f;
  factions[0] = civ;

  // Generate 8-12 procedural factions
  int count = 8 + (rand() % 5);
  for (uint32_t i = 0; i < static_cast<uint32_t>(count); ++i) {
    FactionData data;
    data.id = i + 1; // 0 is reserve/none
    data.name = generateFactionName();
    data.color =
        sf::Color(rand() % 155 + 100, rand() % 155 + 100, rand() % 155 + 100);

    // Randomize weights
    float total = 0.0f;
    data.militaryWeight = (rand() % 100) * 0.01f;
    data.freightWeight = (rand() % 100) * 0.01f;
    data.passengerWeight = (rand() % 100) * 0.01f;
    total = data.militaryWeight + data.freightWeight + data.passengerWeight;
    data.militaryWeight /= total;
    data.freightWeight /= total;
    data.passengerWeight /= total;

    factions[data.id] = data;
  }

  // Initialize neutral relationships
  for (auto const &[idA, _] : factions) {
    for (auto const &[idB, __] : factions) {
      if (idA == idB)
        continue;
      relationships[{std::min(idA, idB), std::max(idA, idB)}] = 0.0f; // Neutral
    }
  }
}

void FactionManager::update(entt::registry &registry, float deltaTime) {
  auto span =
      Telemetry::instance().tracer()->StartSpan("faction.credit.accumulate");
  // 1. Credit generation from controlled planets
  auto view = registry.view<PlanetEconomy, Faction>();
  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    auto &factionComp = view.get<Faction>(entity);

    for (auto const &[factionId, allegiance] : factionComp.allegiances) {
      if (factions.count(factionId)) {
        // Factions get credits based on population and allegiance
        float earnings = (eco.populationCount * allegiance * 0.01f) * deltaTime;
        factions[factionId].credits += earnings;
      }
    }
  }
  // Calculate total credits for observability
  double totalCredits = 0;
  for (auto const &[id, data] : factions) {
    totalCredits += data.credits;
  }
  span->SetAttribute("faction.total_credits", totalCredits);
  span->SetAttribute("faction.count", (int)factions.size());
  span->End();
}

const FactionData &FactionManager::getFaction(uint32_t id) const {
  static FactionData nullFaction = {0, "Independent", sf::Color::White};
  auto it = factions.find(id);
  if (it != factions.end())
    return it->second;
  return nullFaction;
}

FactionData *FactionManager::getFactionPtr(uint32_t id) {
  auto it = factions.find(id);
  if (it != factions.end())
    return &it->second;
  return nullptr;
}

uint32_t FactionManager::getRandomFactionId() const {
  if (factions.empty())
    return 0;
  auto it = factions.begin();
  std::advance(it, rand() % factions.size());
  return it->first;
}

float FactionManager::getRelationship(uint32_t idA, uint32_t idB) const {
  if (idA == idB)
    return 1.0f;
  auto it = relationships.find({std::min(idA, idB), std::max(idA, idB)});
  if (it != relationships.end())
    return it->second;
  return 0.0f;
}

void FactionManager::adjustRelationship(uint32_t idA, uint32_t idB,
                                        float delta) {
  if (idA == idB)
    return;
  auto key = std::make_pair(std::min(idA, idB), std::max(idA, idB));
  relationships[key] = std::clamp(relationships[key] + delta, -1.0f, 1.0f);
}

std::string FactionManager::generateFactionName() {
  static const std::vector<std::string> adjectives = {
      "Nova",  "Void", "Galactic", "United",    "Crimson",
      "Solar", "Deep", "Zenith",   "Forbidden", "Iron"};
  static const std::vector<std::string> nouns = {
      "Syndicate", "Alliance", "Hegemony", "Collective", "Directorate",
      "Cartel",    "Dominion", "Order",    "Vanguard",   "Remnant"};

  return adjectives[rand() % adjectives.size()] + " " +
         nouns[rand() % nouns.size()];
}

} // namespace space
