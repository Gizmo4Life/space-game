#include "FactionManager.h"
#include "engine/telemetry/Telemetry.h"
#include "game/ShipOutfitter.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/FactionDNA.h"
#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <opentelemetry/trace/provider.h>
#include <random>

// DNA drift events are recorder in EconomyManager.

namespace space {

void FactionManager::init() {
  factions.clear();
  relationships.clear();

  auto generateShipLines = [&](FactionData &fData) {
    fData.blueprints.clear();
    for (Tier t : {Tier::T1, Tier::T2, Tier::T3}) {
      const TierDNA &tdna = fData.dna.tierDNA.at(t);

      // Number of lines scales with specialization (lower = more variety)
      // and fleetScale (higher = more investment in the tier)
      int numLines = 1;
      if (tdna.specialization < 0.4f)
        numLines += 1;
      if (tdna.fleetScale > 0.7f)
        numLines += 1;

      // Ensure at least one Combat and one Commercial line if appropriate
      std::vector<std::string> roles;
      if (fData.dna.aggression > 0.3f)
        roles.push_back("Combat");
      if (fData.dna.commercialism > 0.3f)
        roles.push_back("Cargo");
      if (roles.empty())
        roles.push_back("General");

      for (int i = 0; i < numLines; ++i) {
        std::string role = roles[i % roles.size()];
        if (i >= (int)roles.size())
          role += " " + std::to_string(i / roles.size() + 1);

        fData.blueprints.push_back(
            ShipOutfitter::instance().generateBlueprint(fData.id, t, role, i));
      }
    }
  };

  // Faction 0 is Neutral
  FactionData civ;
  civ.id = 0;
  civ.name = "Civilian";
  civ.color = sf::Color(150, 150, 150);
  civ.dna.aggression = 0.05f;
  civ.dna.industrialism = 1.0f;
  civ.dna.commercialism = 1.0f;
  civ.dna.cooperation = 1.0f;
  for (Tier t : {Tier::T1, Tier::T2, Tier::T3}) {
    TierDNA &tdna = civ.dna.tierDNA[t];
    tdna.fleetScale = (t == Tier::T1) ? 0.8f : 0.2f;
    tdna.specialization = 0.5f;
    tdna.hardpointDensities[Tier::T1] = 0.2f;
    tdna.mountDensities[Tier::T1] = 0.4f;
    tdna.prefVolume = 0.9f;
    tdna.prefDurability = 0.2f;
  }
  civ.dna.visual.bodyStyle = VisualStyle::Square;
  civ.dna.visual.nacelleStyle = NacelleStyle::Integrated;
  civ.dna.visual.layoutPattern = LayoutPattern::Symmetrical;
  civ.dna.namingScheme = NamingScheme::Avian;
  generateShipLines(civ);
  factions[0] = civ;

  // Faction 1 is Player
  FactionData player;
  player.id = 1;
  player.name = "Player";
  player.color = sf::Color::Cyan;
  player.dna.aggression = 0.5f;
  player.dna.industrialism = 0.5f;
  player.dna.commercialism = 0.5f;
  player.dna.cooperation = 0.5f;
  player.dna.namingScheme = NamingScheme::Raptors;
  player.dna.visual.layoutPattern = LayoutPattern::Symmetrical;
  player.dna.visual.nacelleStyle =
      NacelleStyle::Integrated; // Polygons look better integrated
  player.dna.visual.bodyStyle = VisualStyle::Polygon;
  for (Tier t : {Tier::T1, Tier::T2, Tier::T3}) {
    TierDNA &tdna = player.dna.tierDNA[t];
    tdna.fleetScale = 0.5f;
    tdna.specialization = 0.5f;
    tdna.prefDurability = 0.5f;
    tdna.prefVolume = 0.4f;
    tdna.hardpointDensities[Tier::T1] = 0.5f;
    tdna.hardpointDensities[Tier::T2] = 0.3f;
    tdna.mountDensities[Tier::T1] = 0.5f;
    tdna.mountDensities[Tier::T2] = 0.3f;
  }
  generateShipLines(player);
  factions[1] = player;

  // Generate exactly 5 procedural factions (IDs 2–6)
  int count = 5;
  for (uint32_t i = 0; i < static_cast<uint32_t>(count); ++i) {
    FactionData data;
    data.id = i + 2;
    data.name = generateFactionName();
    data.color = sf::Color(rand() % 155 + 100, rand() % 155 + 100,
                           rand() % 155 + 100, 255);

    data.dna.aggression = (rand() % 100) * 0.01f;
    data.dna.industrialism = (rand() % 100) * 0.01f;
    data.dna.commercialism = (rand() % 100) * 0.01f;
    data.dna.cooperation = (rand() % 100) * 0.01f;
    data.dna.namingScheme = static_cast<NamingScheme>(rand() % 11);

    data.dna.visual.layoutPattern = static_cast<LayoutPattern>(rand() % 4);
    data.dna.visual.nacelleStyle = static_cast<NacelleStyle>(rand() % 4);
    data.dna.visual.hullConnectivity =
        static_cast<HullConnectivity>(rand() % 3);
    // User preference: default to Polygon for better aesthetics, but allow
    // variety
    data.dna.visual.bodyStyle = VisualStyle::Polygon;

    for (Tier t : {Tier::T1, Tier::T2, Tier::T3}) {
      TierDNA &tdna = data.dna.tierDNA[t];
      tdna.fleetScale = (rand() % 100) * 0.01f;
      tdna.specialization = (rand() % 100) * 0.01f;
      tdna.prefDurability = (rand() % 100) * 0.01f;
      tdna.prefVolume = (rand() % 100) * 0.01f;

      for (Tier mt : {Tier::T1, Tier::T2, Tier::T3}) {
        tdna.hardpointDensities[mt] = (rand() % 50) * 0.01f;
        tdna.mountDensities[mt] = (rand() % 50) * 0.01f;
      }
    }

    generateShipLines(data);
    factions[data.id] = data;
  }

  // Initialize relationships: alignment-based fuzzy logic
  for (auto const &[idA, dataA] : factions) {
    for (auto const &pairB : factions) {
      uint32_t idB = pairB.first;
      const FactionData &dataB = pairB.second;
      if (idA >= idB)
        continue;

      // Calculate alignment: similar DNA = better starting relationship
      float aggDiff = std::abs(dataA.dna.aggression - dataB.dna.aggression);
      float indDiff =
          std::abs(dataA.dna.industrialism - dataB.dna.industrialism);
      float comDiff =
          std::abs(dataA.dna.commercialism - dataB.dna.commercialism);
      float coopDiff = std::abs(dataA.dna.cooperation - dataB.dna.cooperation);

      float avgDiff = (aggDiff + indDiff + comDiff + coopDiff) / 4.0f;
      float alignment = 1.0f - avgDiff;

      // Cooperation scales how much alignment matters
      float baseRel = (alignment - 0.5f) * 2.0f; // -1.0 to 1.0
      float multiplier = (dataA.dna.cooperation + dataB.dna.cooperation) * 0.5f;
      float startRel = baseRel * multiplier;

      // Clamp and set
      startRel = std::max(-1.0f, std::min(1.0f, startRel));
      relationships[{idA, idB}] = startRel;
      relationships[{idB, idA}] = startRel;
    }
  }
}

void FactionManager::update(entt::registry &registry, float deltaTime) {
  static float evalTimer = 10.0f;
  evalTimer -= deltaTime;
  if (evalTimer <= 0) {
    evalTimer = 30.0f; // Re-evaluate every 30s

    for (auto &pair : factions) {
      uint32_t id = pair.first;
      FactionData &data = pair.second;
      if (id == 0)
        continue; // Skip Civilian?

      float kdRatio = data.stats.getGlobalKillDeathValueRatio();

      // --- DNA Drift / Evolution ---
      // If K/D is low (< 0.8), we are losing value. Shift towards
      // commercialism/industrialism to fund more ships, or shift Aggression to
      // find better targets.
      if (kdRatio < 0.8f) {
        data.dna.aggression = std::max(0.0f, data.dna.aggression - 0.05f);
        data.dna.industrialism = std::min(1.0f, data.dna.industrialism + 0.05f);
      } else if (kdRatio > 1.5f) {
        // We are winning! Become more aggressive.
        data.dna.aggression = std::min(1.0f, data.dna.aggression + 0.05f);
      }

      // --- Economic Re-evaluation ---
      // Use Fuzzy Logic (DNA weights) to decide next factory build
      // We'll simulate this by occasionally unlocking tech or adjusting credits
      std::random_device rd;
      std::mt19937 g(rd());
      std::uniform_real_distribution<float> dist(0, 1);

      if (dist(g) < data.dna.industrialism * 0.1f) {
        data.credits +=
            1000.0f; // Industrial factions "generate" more wealth/resources
      }
    }
  }
  auto span =
      Telemetry::instance().tracer()->StartSpan("faction.credit.accumulate");
  // 1. Credit generation from controlled planets
  double totalCredits = 0;
  auto view = registry.view<PlanetEconomy, Faction>();
  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    auto &factionComp = view.get<Faction>(entity);

    for (auto const &pair : eco.factionData) {
      uint32_t factionId = pair.first;
      const FactionEconomy &fEco = pair.second;
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
  for (auto const &pair : factions) {
    uint32_t id = pair.first;
    const FactionData &data = pair.second;
    totalCredits += data.credits;
  }

  span->SetAttribute("faction.total_credits", totalCredits);
  span->SetAttribute("faction.count", (int)factions.size());

  // 2. Relationship Decay (moves towards 0)
  float decayRate = 0.005f; // Very slow decay
  for (auto &pair : relationships) {
    float &rel = pair.second;
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
      std::cout << "[Faction] Relationship decay: (" << pair.first.first << ","
                << pair.first.second << ") " << oldRel << " -> " << rel << "\n";
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
      "Obsidian", "Storm",   "Iron",     "Swift",  "Cloud",
      "Steel",    "Apex",    "Primal",   "Modern", "Shadow",
      "Gale",     "Granite", "Resolute", "Stable", "Active"};
  static const std::vector<std::string> nouns = {
      "Collective",  "Compact", "Syndicate", "Alliance", "Hegemony",
      "Directorate", "Cartel",  "Dominion",  "Order",    "Vanguard",
      "Consortium",  "Systems", "Dynamics",  "Works",    "Forge"};

  return adjectives[rand() % adjectives.size()] + " " +
         nouns[rand() % nouns.size()];
}

std::string FactionManager::generateShipLineName(NamingScheme scheme,
                                                 uint32_t index) {
  static const std::map<NamingScheme, std::vector<std::string>> schemeNames = {
      {NamingScheme::Raptors,
       {"Eagle", "Hawk", "Falcon", "Osprey", "Kestrel", "Merlin", "Goshawk",
        "Buzzard", "Harrier", "Vulture"}},
      {NamingScheme::Rodents,
       {"Mouse", "Rat", "Squirrel", "Chipmunk", "Beaver", "Gopher", "Marmot",
        "Capybara", "Porcupine", "Jerboa"}},
      {NamingScheme::Ungulates,
       {"Gazelle", "Impala", "Kudu", "Oryx", "Eland", "Bongo", "Okapi",
        "Giraffe", "Zebra", "Hippo"}},
      {NamingScheme::Insects,
       {"Hornet", "Wasp", "Beetle", "Mantis", "Locust", "Dragonfly", "Scorpion",
        "Spider", "Ant", "Moth"}},
      {NamingScheme::Felines,
       {"Tiger", "Lion", "Leopard", "Jaguar", "Cougar", "Lynx", "Ocelot",
        "Caracal", "Serval", "Cheetah"}},
      {NamingScheme::Mythical,
       {"Dragon", "Phoenix", "Griffin", "Hydra", "Chimera", "Basilisk",
        "Kraken", "Sphinx", "Wyvern", "Gorgon"}},
      {NamingScheme::Celestial,
       {"Pulsar", "Quasar", "Nebula", "Comet", "Meteor", "Asteroid", "Galaxy",
        "Zenith", "Nadir", "Eclipse"}},
      {NamingScheme::Avian,
       {"Sparrow", "Falcon", "Eagle", "Raven", "Owl", "Swift", "Kestrel",
        "Merlin", "Harrier", "Vulture"}},
      {NamingScheme::Geological,
       {"Basalt", "Granite", "Obsidian", "Quartz", "Shale", "Flint", "Marble",
        "Slate", "Chert", "Gneiss"}},
      {NamingScheme::Meteorological,
       {"Storm", "Gale", "Tempest", "Cyclone", "Typhoon", "Blizzard", "Thunder",
        "Frost", "Mist", "Haze"}},
      {NamingScheme::Mechanical,
       {"Gear", "Piston", "Dynamo", "Turbine", "Valve", "Crank", "Bellows",
        "Spring", "Toggle", "Ratchet"}}};

  auto it = schemeNames.find(scheme);
  if (it == schemeNames.end())
    return "Unknown";

  const auto &names = it->second;
  return names[index % names.size()];
}

const ShipBlueprint *FactionData::getBlueprint(Tier tier,
                                               const std::string &role,
                                               uint32_t lineIndex) const {
  for (const auto &bp : blueprints) {
    if (bp.hull.sizeTier == tier && bp.role == role &&
        bp.lineIndex == lineIndex)
      return &bp;
  }
  return nullptr;
}

} // namespace space
