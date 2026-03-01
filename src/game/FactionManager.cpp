#include "FactionManager.h"
#include "engine/telemetry/Telemetry.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <opentelemetry/trace/provider.h>

namespace space {

void FactionManager::init() {
  factions.clear();
  relationships.clear();

  // Faction 0 is Neutral
  FactionData civ;
  civ.id = 0;
  civ.name = "Civilian";
  civ.color = sf::Color(150, 150, 150);
  civ.militaryWeight = 0.05f;
  civ.freightWeight = 0.45f;
  civ.passengerWeight = 0.5f;
  factions[0] = civ;

  // Faction 1 is Player
  FactionData player;
  player.id = 1;
  player.name = "Player";
  player.color = sf::Color::Cyan;
  factions[1] = player;

  // Generate exactly 5 procedural factions (IDs 2–6)
  int count = 5;
  for (uint32_t i = 0; i < static_cast<uint32_t>(count); ++i) {
    FactionData data;
    data.id = i + 2; // Start from 2
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

  // Initialize relationships: most neutral, some allies, some rivals
  for (auto const &[idA, _] : factions) {
    for (auto const &[idB, __] : factions) {
      if (idA >= idB)
        continue;

      float rel = 0.0f;
      int roll = rand() % 100;
      if (roll < 5)
        rel = 1.0f; // 5% chance of being total Allies
      else if (roll < 10)
        rel = -1.0f; // 5% chance of being Mortal Enemies
      else if (roll < 20)
        rel = -0.5f; // 10% chance of being Rivals
      else if (roll < 30)
        rel = 0.5f; // 10% chance of being Friendly

      relationships[{idA, idB}] = rel;
    }
  }
}

void FactionManager::update(entt::registry &registry, float deltaTime) {
  auto span =
      Telemetry::instance().tracer()->StartSpan("faction.credit.accumulate");
  // 1. Credit generation from controlled planets
  double totalCredits = 0;
  auto view = registry.view<PlanetEconomy, Faction>();
  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    auto &factionComp = view.get<Faction>(entity);

    for (auto const &[factionId, fEco] : eco.factionData) {
      if (factions.count(factionId)) {
        // Factions get credits based on their specific population on the planet
        float earnings = (fEco.populationCount * 0.01f) * deltaTime;
        factions[factionId].credits += earnings;
        totalCredits += earnings; // This is earnings per tick, but let's track
                                  // total holdings
      }
    }
  }

  // Recalculate total holdings for telemetry
  totalCredits = 0;
  for (auto const &[id, data] : factions) {
    totalCredits += data.credits;
  }

  span->SetAttribute("faction.total_credits", totalCredits);
  span->SetAttribute("faction.count", (int)factions.size());

  // 2. Relationship Decay (moves towards 0)
  float decayRate = 0.005f; // Very slow decay
  for (auto &[pair, rel] : relationships) {
    float oldRel = rel;
    if (rel > 0.01f) {
      rel = std::max(0.0f, rel - decayRate * deltaTime);
    } else if (rel < -0.01f) {
      rel = std::min(0.0f, rel + decayRate * deltaTime);
    } else {
      rel = 0.0f;
    }

    // Periodically log significant decay (limit output noise)
    static float logTimer = 0.0f;
    logTimer += deltaTime;
    if (logTimer > 20.0f && std::abs(oldRel - rel) > 0.0001f) {
      std::cout << "[Faction] Relationship decay: (" << pair.first << ","
                << pair.second << ") " << oldRel << " -> " << rel << "\n";
      logTimer = 0.0f;
    }
  }

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
  std::cout << "[Faction] Relationship between " << idA << " and " << idB
            << " adjusted by " << delta << " to " << relationships[key] << "\n";
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
