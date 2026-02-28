#include "NPCShipManager.h"
#include "engine/combat/WeaponSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace space {

// ─── Init ───────────────────────────────────────────────────────────────
void NPCShipManager::init(b2WorldId worldId) {
  worldId_ = worldId;
  spawnTimer_ = 2.0f; // First spawn after 2s
  initialized_ = true;
  std::cout << "[NPC] Manager initialized\n";
}

// ─── Main update ────────────────────────────────────────────────────────
void NPCShipManager::update(entt::registry &registry, float deltaTime) {
  auto span = Telemetry::instance().tracer()->StartSpan("npc.ai.tick");

  // Continuous spawning
  if (initialized_) {
    spawnTimer_ -= deltaTime;
    if (spawnTimer_ <= 0) {
      // Calculate total population to adjust spawn interval
      float totalPop = 0.0f;
      auto ecoView = registry.view<PlanetEconomy>();
      for (auto e : ecoView) {
        totalPop += ecoView.get<PlanetEconomy>(e).getTotalPopulation();
      }

      // Base interval is 4s (SPAWN_INTERVAL) at 5k pop, scales down
      // significantly 10x Density Boost
      float popFactor = std::clamp(totalPop / 5000.0f, 1.0f, 100.0f) * 10.0f;
      spawnTimer_ = SPAWN_INTERVAL / popFactor;

      // Count current NPCs
      int count = 0;
      auto countView = registry.view<NPCComponent>();
      for ([[maybe_unused]] auto e : countView)
        count++;

      if (count < MAX_NPCS) {
        spawnAtRandomPlanet(registry);
      }
    }
  }

  // Tick AI for all NPCs
  tickAI(registry, deltaTime);

  // Report count
  int npcCount = 0;
  auto cv = registry.view<NPCComponent>();
  for ([[maybe_unused]] auto e : cv)
    npcCount++;

  span->SetAttribute("npc.active_count", npcCount);
  span->End();
}

// ─── Spawn at a random planet ───────────────────────────────────────────
void NPCShipManager::spawnAtRandomPlanet(entt::registry &registry) {
  auto planet = pickRandomPlanet(registry);
  auto &planetTrans = registry.get<TransformComponent>(planet);

  // Pick a faction based on planet's allegiance
  uint32_t factionId = 0; // Default to Civilian
  if (registry.all_of<Faction>(planet)) {
    auto &fComp = registry.get<Faction>(planet);
    float roll = (rand() % 100) * 0.01f;
    float accum = 0.0f;
    for (auto const &[fId, weight] : fComp.allegiances) {
      accum += weight;
      if (roll <= accum) {
        factionId = fId;
        break;
      }
    }
  }

  // Spawn near the planet (slight offset so ships don't overlap)
  // --- Fleet Pool Check ---
  auto &fMgr = FactionManager::instance();
  const FactionData &fData = fMgr.getFaction(factionId);
  auto &eco = registry.get<PlanetEconomy>(planet);
  VesselType vTypeReq;
  float roll = (rand() % 100) * 0.01f;
  if (roll < fData.militaryWeight)
    vTypeReq = VesselType::Military;
  else if (roll < fData.militaryWeight + fData.freightWeight)
    vTypeReq = VesselType::Freight;
  else
    vTypeReq = VesselType::Passenger;

  if (eco.factionData[factionId].fleetPool[vTypeReq] <= 0) {
    return;
  }

  // --- Valid Spawn: Perform it once ---
  float angle = static_cast<float>(rand()) / RAND_MAX * 6.28f;
  float offset = 30.0f + static_cast<float>(rand() % 50);
  float bx = planetTrans.position.x / WorldConfig::WORLD_SCALE +
             std::cos(angle) * offset;
  float by = planetTrans.position.y / WorldConfig::WORLD_SCALE +
             std::sin(angle) * offset;
  sf::Vector2f spawnPos(bx, by);

  auto entity = spawnShip(registry, factionId, spawnPos, worldId_);

  if (registry.valid(entity)) {
    auto &npc = registry.get<NPCComponent>(entity);
    npc.homePlanet = planet;
    npc.vesselType = vTypeReq;

    // Decrement pool
    eco.factionData[factionId].fleetPool[vTypeReq]--;

    if (vTypeReq == VesselType::Military) {
      npc.belief = AIBelief::Escort;
      registry.get<NameComponent>(entity).name = fData.name + " Patrol";
    } else if (vTypeReq == VesselType::Freight) {
      npc.belief = AIBelief::Trader;
      registry.get<NameComponent>(entity).name = fData.name + " Freighter";
    } else {
      npc.belief = AIBelief::Trader;
      registry.get<NameComponent>(entity).name = fData.name + " Transport";
    }

    npc.state = AIState::Idle;
    npc.decisionTimer = 0.0f;
  }
}

// ─── Pick a random planet entity ────────────────────────────────────────
entt::entity NPCShipManager::pickRandomPlanet(entt::registry &registry,
                                              entt::entity exclude) {
  std::vector<std::pair<entt::entity, float>> planets;
  float totalWeight = 0.0f;

  auto view = registry.view<PlanetEconomy, TransformComponent>();
  for (auto entity : view) {
    if (entity != exclude) {
      float weight = view.get<PlanetEconomy>(entity).getTotalPopulation();
      planets.push_back({entity, weight});
      totalWeight += weight;
    }
  }
  if (planets.empty() || totalWeight <= 0)
    return entt::null;

  float roll = (rand() % 1000) * 0.001f * totalWeight;
  float accum = 0.0f;
  for (auto const &[entity, weight] : planets) {
    accum += weight;
    if (roll <= accum)
      return entity;
  }
  return planets.back().first;
}

entt::entity NPCShipManager::pickExpansionTarget(entt::registry &registry,
                                                 uint32_t factionId) {
  std::vector<entt::entity> candidates;
  auto view = registry.view<PlanetEconomy>();

  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    bool hasPresence = eco.factionData.count(factionId) > 0 &&
                       !eco.factionData.at(factionId).isAbandoned;
    if (!hasPresence) {
      candidates.push_back(entity);
    }
  }

  if (candidates.empty())
    return entt::null;
  return candidates[rand() % candidates.size()];
}

// ─── AI State Machine ───────────────────────────────────────────────────
void NPCShipManager::tickAI(entt::registry &registry, float dt) {
  auto view = registry.view<NPCComponent, InertialBody>();

  for (auto entity : view) {
    auto &npc = view.get<NPCComponent>(entity);
    auto &inertial = view.get<InertialBody>(entity);

    if (!b2Body_IsValid(inertial.bodyId))
      continue;

    npc.lifeTimer += dt;
    b2Vec2 myPos = b2Body_GetPosition(inertial.bodyId);

    // --- Relationship / Hostility Check ---
    float sensorRange = 100.0f;
    auto targetView = registry.view<Faction, InertialBody>();
    for (auto other : targetView) {
      if (other == entity)
        continue;
      auto &oFaction = targetView.get<Faction>(other);
      auto &oInertial = targetView.get<InertialBody>(other);
      b2Vec2 oPos = b2Body_GetPosition(oInertial.bodyId);

      float dx = myPos.x - oPos.x;
      float dy = myPos.y - oPos.y;
      float dSq = dx * dx + dy * dy;

      if (dSq < sensorRange * sensorRange) {
        float rel = FactionManager::instance().getRelationship(
            npc.factionId, oFaction.getMajorityFaction());
        // Mortal Enemy: Attack unprovoked
        if (rel <= -0.9f) {
          npc.state = AIState::Combat;
          npc.targetEntity = other;
        }
        // Rival: Attack if too close (provoked)
        else if (rel <= -0.4f && dSq < 40.0f * 40.0f) {
          npc.state = AIState::Combat;
          npc.targetEntity = other;
        }
      }
    }

    // --- Boids Steering Forces ---
    b2Vec2 sepForce = {0.0f, 0.0f};
    b2Vec2 alignForce = {0.0f, 0.0f};
    b2Vec2 cohesionForce = {0.0f, 0.0f};

    int sepCount = 0;
    int neighborCount = 0;
    b2Vec2 avgPos = {0.0f, 0.0f};
    b2Vec2 avgVel = {0.0f, 0.0f};

    float sepRadius = 40.0f;
    float neighborRadius = 150.0f;

    for (auto other : view) {
      if (other == entity)
        continue;

      auto &oNPC = view.get<NPCComponent>(other);
      auto &oInertial = view.get<InertialBody>(other);
      b2Vec2 oPos = b2Body_GetPosition(oInertial.bodyId);
      b2Vec2 oVel = b2Body_GetLinearVelocity(oInertial.bodyId);

      float dx = myPos.x - oPos.x;
      float dy = myPos.y - oPos.y;
      float dSq = dx * dx + dy * dy;

      if (dSq < neighborRadius * neighborRadius) {
        float d = std::sqrt(dSq);

        // Separation: Global (avoid all)
        if (d < sepRadius && d > 0.001f) {
          sepForce.x += dx / d;
          sepForce.y += dy / d;
          sepCount++;
        }

        // Alignment & Cohesion: Same-Faction OR Allied OR our Leader
        float relationship = FactionManager::instance().getRelationship(
            npc.factionId, oNPC.factionId);
        bool isAlly = (oNPC.factionId == npc.factionId || relationship > 0.4f);
        bool isLeader = (other == npc.leaderEntity);

        if (isAlly || isLeader) {
          float weight = isLeader ? 4.0f : 1.0f; // Leader has high influence
          avgPos.x += oPos.x * weight;
          avgPos.y += oPos.y * weight;
          avgVel.x += oVel.x * weight;
          avgVel.y += oVel.y * weight;
          neighborCount += weight;
        }
      }
    }

    // --- Player Fleet: Specific Follow Logic ---
    if (npc.isPlayerFleet && npc.leaderEntity != entt::null &&
        registry.valid(npc.leaderEntity)) {
      auto &lInertial = registry.get<InertialBody>(npc.leaderEntity);
      b2Vec2 lPos = b2Body_GetPosition(lInertial.bodyId);
      float ldx = lPos.x - myPos.x;
      float ldy = lPos.y - myPos.y;
      float ldSq = ldx * ldx + ldy * ldy;

      // If very far from leader, steer aggressively back
      if (ldSq > 200.0f * 200.0f) {
        float ld = std::sqrt(ldSq);
        b2Body_ApplyForceToCenter(
            inertial.bodyId,
            {ldx / ld * thrust * 3.0f, ldy / ld * thrust * 3.0f}, true);
      }
    }

    if (sepCount > 0) {
      sepForce.x /= sepCount;
      sepForce.y /= sepCount;
    }
    if (neighborCount > 0) {
      avgPos.x /= neighborCount;
      avgPos.y /= neighborCount;
      avgVel.x /= neighborCount;
      avgVel.y /= neighborCount;

      // Cohesion: steer towards center of mass
      cohesionForce.x = avgPos.x - myPos.x;
      cohesionForce.y = avgPos.y - myPos.y;
      float cohD = std::sqrt(cohesionForce.x * cohesionForce.x +
                             cohesionForce.y * cohesionForce.y);
      if (cohD > 0) {
        cohesionForce.x /= cohD;
        cohesionForce.y /= cohD;
      }

      // Alignment: steer towards average velocity
      float velD = std::sqrt(avgVel.x * avgVel.x + avgVel.y * avgVel.y);
      if (velD > 0) {
        alignForce.x = avgVel.x / velD;
        alignForce.y = avgVel.y / velD;
      }
    }

    // Weight and Apply Boid Forces
    float thrust = inertial.thrustForce;
    b2Vec2 totalBoidForce = {
        (sepForce.x * 1.5f + alignForce.x * 0.8f + cohesionForce.x * 0.3f) *
            thrust,
        (sepForce.y * 1.5f + alignForce.y * 0.8f + cohesionForce.y * 0.3f) *
            thrust};
    b2Body_ApplyForceToCenter(inertial.bodyId, totalBoidForce, true);

    switch (npc.state) {

    // ─── IDLE: pick a destination based on belief ─────────────────
    case AIState::Idle: {
      npc.decisionTimer -= dt;
      if (npc.decisionTimer > 0)
        break;

      switch (npc.belief) {
      case AIBelief::Trader: {
        // --- Expansion & Logistics Logic ---
        if (npc.homePlanet != entt::null && registry.valid(npc.homePlanet)) {
          auto &homeEco = registry.get<PlanetEconomy>(npc.homePlanet);
          auto &fEco = homeEco.factionData[npc.factionId];

          // 1. Colonization (Passenger Ships)
          if (npc.vesselType == VesselType::Passenger &&
              fEco.populationCount > 10.0f && npc.passengerCount <= 0.0f) {
            auto dest = pickExpansionTarget(registry, npc.factionId);
            if (dest != entt::null) {
              float moveAmt = std::min(100.0f, fEco.populationCount * 0.1f);
              fEco.populationCount -= moveAmt;
              npc.passengerCount = moveAmt;
              npc.targetEntity = dest;
              npc.state = AIState::Traveling;
              std::cout << "[NPC] Colony Ship " << npc.factionId << " moving "
                        << (moveAmt * 1000.0f) << " people to "
                        << (uint32_t)dest << "\n";
              break;
            }
          }

          // 2. Logistics (Freighters)
          if (npc.vesselType == VesselType::Freight) {
            float buffer = fEco.populationCount * 0.5f;
            Resource surplusRes = (Resource)-1;
            for (auto const &[res, amt] : fEco.stockpile) {
              if (amt > buffer * 1.5f) {
                surplusRes = res;
                break;
              }
            }

            if (surplusRes != (Resource)-1) {
              auto dest = pickRandomPlanet(registry, npc.homePlanet);
              if (dest != entt::null) {
                auto &cargo = registry.get<CargoComponent>(entity);
                float moveAmt = std::min(cargo.maxCapacity,
                                         fEco.stockpile[surplusRes] - buffer);
                if (cargo.add(surplusRes, moveAmt)) {
                  fEco.stockpile[surplusRes] -= moveAmt;
                  npc.targetEntity = dest;
                  npc.state = AIState::Traveling;
                  std::cout << "[NPC] Logistics " << npc.factionId << " moving "
                            << moveAmt << " " << getResourceName(surplusRes)
                            << "\n";
                  break;
                }
              }
            }
          }

          // 3. Construction (Any ship with Mfg Goods)
          auto &cargo = registry.get<CargoComponent>(entity);
          if (cargo.inventory[Resource::ManufacturingGoods] >= 100.0f) {
            // Pick a planet to build at
            auto dest = pickRandomPlanet(registry);
            if (dest != entt::null) {
              auto &destEco = registry.get<PlanetEconomy>(dest);
              if (destEco.factionData.count(npc.factionId)) {
                // Find a factory we lack
                Resource toBuild = (Resource)-1;
                std::vector<Resource> possible = {
                    Resource::Food, Resource::Fuel, Resource::Electronics,
                    Resource::Shipyard, Resource::Refinery};
                for (auto r : possible) {
                  if (destEco.factionData[npc.factionId].factories[r] < 1) {
                    toBuild = r;
                    break;
                  }
                }

                if (toBuild != (Resource)-1) {
                  npc.constructionTarget = toBuild;
                  npc.targetEntity = dest;
                  npc.state = AIState::Traveling;
                  std::cout << "[NPC] Construction mission: Build "
                            << getResourceName(toBuild) << " at "
                            << (uint32_t)dest << "\n";
                  break;
                }
              }
            }
          }
        }

        // Default: Pick a random planet to travel to (different from home)
        auto dest = pickRandomPlanet(registry, npc.homePlanet);
        if (dest != entt::null) {
          npc.targetEntity = dest;
          npc.state = AIState::Traveling;
        }
        break;
      }
      case AIBelief::Raider: {
        // Raiders also travel to random planets looking for prey
        auto dest = pickRandomPlanet(registry);
        if (dest != entt::null) {
          npc.targetEntity = dest;
          npc.state = AIState::Traveling;
        }
        break;
      }
      case AIBelief::Escort: {
        // Escorts patrol near home planet
        if (npc.homePlanet != entt::null && registry.valid(npc.homePlanet)) {
          npc.targetEntity = npc.homePlanet;
          npc.state = AIState::Traveling;
        }
        break;
      }
      }
      break;
    }

    // ─── TRAVELING: move toward target ────────────────────────────
    case AIState::Traveling: {
      if (npc.targetEntity == entt::null || !registry.valid(npc.targetEntity)) {
        npc.state = AIState::Idle;
        npc.decisionTimer = 1.0f;
        break;
      }

      // Use physics coordinates (meters) for distance
      b2Vec2 targetPhys;
      if (registry.all_of<TransformComponent>(npc.targetEntity)) {
        sf::Vector2f tPixel =
            registry.get<TransformComponent>(npc.targetEntity).position;
        targetPhys = {tPixel.x / WorldConfig::WORLD_SCALE,
                      tPixel.y / WorldConfig::WORLD_SCALE};
      } else if (registry.all_of<InertialBody>(npc.targetEntity)) {
        targetPhys = b2Body_GetPosition(
            registry.get<InertialBody>(npc.targetEntity).bodyId);
      } else {
        npc.state = AIState::Idle;
        break;
      }

      b2Vec2 diff = {targetPhys.x - myPos.x, targetPhys.y - myPos.y};
      float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

      // --- Combat & Evasive Logic ---
      // Only apply to ships/players, not planets or asteroids
      bool isShip = registry.all_of<InertialBody>(npc.targetEntity) &&
                    !registry.all_of<CelestialBody>(npc.targetEntity);

      if (isShip) {
        auto &tInertial = registry.get<InertialBody>(npc.targetEntity);
        b2Vec2 tVel = b2Body_GetLinearVelocity(tInertial.bodyId);

        // Lead Pursuit Calculation
        // TargetPos_Lead = TargetPos + TargetVel * (Distance / ProjectileSpeed)
        float projectileSpeed = 5000.0f; // Matches WeaponComponent
        float timeToTarget = dist / projectileSpeed;
        b2Vec2 leadPos = {targetPhys.x + tVel.x * timeToTarget,
                          targetPhys.y + tVel.y * timeToTarget};

        b2Vec2 leadDiff = {leadPos.x - myPos.x, leadPos.y - myPos.y};
        float leadAngle = std::atan2(leadDiff.y, leadDiff.x);

        // Maintain distance (Evasive)
        // Use lifeTimer and phaseOffset for persistent varying distance
        float phaseShift = std::sin(npc.lifeTimer * 0.2f + npc.phaseOffset);
        float minCombatDist = 250.0f + 100.0f * phaseShift; // 150m to 350m

        if (dist < minCombatDist) {
          // Back off or circle if too close
          // Direction changes every 10-20 seconds for variety
          bool orbitClockwise =
              std::sin(npc.lifeTimer * 0.1f + npc.phaseOffset) > 0;
          leadAngle += 1.57f * (orbitClockwise ? 1.0f : -1.0f);
        }

        // Apply thrust toward lead point
        float thrust = inertial.thrustForce;
        b2Vec2 force = {std::cos(leadAngle) * thrust,
                        std::sin(leadAngle) * thrust};
        b2Body_ApplyForceToCenter(inertial.bodyId, force, true);

        // Rotate to face lead position
        b2Rot targetRot = b2MakeRot(std::atan2(leadDiff.y, leadDiff.x));
        b2Body_SetTransform(inertial.bodyId, myPos, targetRot);

        // Fire if aligned with lead position
        b2Rot myRot = b2Body_GetRotation(inertial.bodyId);
        float myAngle = std::atan2(myRot.s, myRot.c);
        float angleErr = std::abs(myAngle - std::atan2(leadDiff.y, leadDiff.x));
        if (angleErr > 3.14159f)
          angleErr = std::abs(angleErr - 6.28318f);

        if (angleErr < 0.15f && dist < 1500.0f) {
          WeaponSystem::fire(registry, entity, worldId_);
        }
      } else {
        // Standard non-combat movement
        float angle = std::atan2(diff.y, diff.x);
        float thrust = inertial.thrustForce;
        b2Vec2 force = {std::cos(angle) * thrust, std::sin(angle) * thrust};
        b2Body_ApplyForceToCenter(inertial.bodyId, force, true);

        b2Rot targetRot = b2MakeRot(angle);
        b2Body_SetTransform(inertial.bodyId, myPos, targetRot);
      }
      break;
    }

    // ─── DOCKED: wait, then go idle ──────────────────────────────
    case AIState::Docked: {
      npc.dockTimer -= dt;

      // --- Expansion & Logistics Execution ---
      if (npc.targetEntity != entt::null &&
          registry.all_of<PlanetEconomy>(npc.targetEntity)) {
        auto &planetEco = registry.get<PlanetEconomy>(npc.targetEntity);

        // 1. Colonization & Founding
        if (npc.passengerCount > 0.0f) {
          auto &fEco = planetEco.factionData[npc.factionId];
          if (fEco.isAbandoned ||
              planetEco.factionData.count(npc.factionId) == 0) {
            // Founding event
            fEco.isAbandoned = false;
            fEco.strategy = (FactionStrategy)(rand() % 3);
            fEco.credits = npc.passengerCount * 100.0f;
            std::cout << "[NPC] Faction " << npc.factionId
                      << " founded new outpost on "
                      << (uint32_t)npc.targetEntity << " with strategy "
                      << (int)fEco.strategy << " and " << fEco.credits
                      << " credits\n";
          }
          fEco.populationCount += npc.passengerCount;
          npc.passengerCount = 0.0f;
          std::cout << "[NPC] Colonized: New pop " << fEco.populationCount
                    << "\n";
        }

        // 2. Logistics (Unloading)
        auto &cargo = registry.get<CargoComponent>(entity);
        if (cargo.currentWeight > 0.0f) {
          auto &fEco = planetEco.factionData[npc.factionId];
          for (auto const &[res, amt] : cargo.inventory) {
            fEco.stockpile[res] += amt;
          }
          cargo.inventory.clear();
          cargo.currentWeight = 0.0f;
          std::cout << "[NPC] Logistics: Unloaded cargo at destination\n";
        }

        // 3. Factory Construction
        if (npc.constructionTarget != (Resource)-1) {
          auto &cargo = registry.get<CargoComponent>(entity);
          if (cargo.inventory[Resource::ManufacturingGoods] >= 100.0f) {
            auto &fEco = planetEco.factionData[npc.factionId];
            fEco.factories[npc.constructionTarget]++;
            cargo.remove(Resource::ManufacturingGoods, 100.0f);
            std::cout << "[NPC] Construction Complete: "
                      << getResourceName(npc.constructionTarget) << " at "
                      << (uint32_t)npc.targetEntity << "\n";
            npc.constructionTarget = (Resource)-1;
          }
        }

        // --- Abandoned Outpost Claiming (Scavenging) ---
        for (auto &[fId, fEco] : planetEco.factionData) {
          if (fEco.isAbandoned && fId != npc.factionId) {
            // Claim the stockpile
            auto &myEco = planetEco.factionData[npc.factionId];
            for (auto const &[res, amt] : fEco.stockpile) {
              myEco.stockpile[res] += amt;
            }
            fEco.stockpile.clear();
            std::cout << "[NPC] Faction " << npc.factionId
                      << " claimed abandoned outpost of faction " << fId
                      << "\n";
          }
        }
      }

      if (npc.belief == AIBelief::Escort && npc.homePlanet != entt::null &&
          registry.valid(npc.homePlanet)) {
        // Regional Patrol: sweep the area around the planet
        auto &homeTrans =
            registry.get<TransformComponent>(npc.homePlanet).position;

        // 1. Angular motion with "zigzag" variation
        // Use dockTimer to create a varying angular velocity
        float angularVel = 0.15f + 0.1f * std::sin(npc.dockTimer * 0.5f);
        npc.patrolAngle += dt * angularVel;

        // 2. Oscillating distance (Regional Sweep)
        // Moves between 600 and 1500 units from the planet
        // All calculations in physics meters
        float baseDist = 1050.0f;
        float amp = 450.0f;
        float distOsc = amp * std::sin(npc.dockTimer * 0.2f + npc.phaseOffset);
        float patrolDist = baseDist + distOsc;

        b2Vec2 homePos = {homeTrans.x / WorldConfig::WORLD_SCALE,
                          homeTrans.y / WorldConfig::WORLD_SCALE};
        float px = homePos.x + std::cos(npc.patrolAngle) * patrolDist;
        float py = homePos.y + std::sin(npc.patrolAngle) * patrolDist;

        // 3. Move toward the sweeping target point
        float dx = px - myPos.x;
        float dy = py - myPos.y;
        float d = std::sqrt(dx * dx + dy * dy);
        if (d > 0.5f) {
          float thrust =
              inertial.thrustForce * 0.5f; // Active thrust for patrolling
          b2Vec2 force = {(dx / d) * thrust, (dy / d) * thrust};
          b2Body_ApplyForceToCenter(inertial.bodyId, force, true);
        }
      }

      if (npc.dockTimer <= 0) {
        npc.state = AIState::Idle;
        npc.decisionTimer = 0.5f;
        // After docking, update home planet to current target (traders move
        // along)
        if (npc.belief == AIBelief::Trader && npc.targetEntity != entt::null) {
          npc.homePlanet = npc.targetEntity;
        }
        npc.targetEntity = entt::null;
      }
      break;
    }

    // ─── COMBAT: Engage target ──────────────────────────
    case AIState::Combat: {
      if (npc.targetEntity == entt::null || !registry.valid(npc.targetEntity)) {
        npc.state = AIState::Idle;
        npc.decisionTimer = 1.0f;
        break;
      }

      auto &tInertial = registry.get<InertialBody>(npc.targetEntity);
      b2Vec2 targetPos = b2Body_GetPosition(tInertial.bodyId);
      b2Vec2 tVel = b2Body_GetLinearVelocity(tInertial.bodyId);

      b2Vec2 toTarget = {targetPos.x - myPos.x, targetPos.y - myPos.y};
      float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

      if (dist > 300.0f) { // Lost target
        npc.state = AIState::Idle;
        npc.targetEntity = entt::null;
        break;
      }

      // Lead Pursuit
      float projectileSpeed = 40.0f;
      float timeToReach = dist / projectileSpeed;
      b2Vec2 predictedPos = {targetPos.x + tVel.x * timeToReach,
                             targetPos.y + tVel.y * timeToReach};

      b2Vec2 steerDir = {predictedPos.x - myPos.x, predictedPos.y - myPos.y};
      float steerDist =
          std::sqrt(steerDir.x * steerDir.x + steerDir.y * steerDir.y);
      if (steerDist > 0.1f) {
        steerDir.x /= steerDist;
        steerDir.y /= steerDist;
      }

      // Thrust & Rotate
      float thrust = inertial.thrustForce;
      if (dist > 100.0f) {
        b2Body_ApplyForceToCenter(
            inertial.bodyId, {steerDir.x * thrust, steerDir.y * thrust}, true);
      }

      // Face the predicted position
      float targetAngle = std::atan2(steerDir.y, steerDir.x);
      b2Body_SetTransform(inertial.bodyId, myPos, b2MakeRot(targetAngle));

      // Fire if close enough and facing roughly right way
      if (dist < 150.0f && registry.all_of<WeaponComponent>(entity)) {
        WeaponSystem::fire(registry, entity, worldId_);
      }
      break;
    }
    case AIState::Fleeing:
      npc.state = AIState::Idle;
      npc.decisionTimer = 2.0f;
      break;
    }
  }
}

// ─── Spawn ship entity ──────────────────────────────────────────────────
entt::entity NPCShipManager::spawnShip(entt::registry &registry,
                                       uint32_t factionId,
                                       sf::Vector2f position, b2WorldId worldId,
                                       bool isPlayerFleet,
                                       entt::entity leaderEntity) {
  auto span = Telemetry::instance().tracer()->StartSpan("npc.spawn");
  span->SetAttribute("npc.faction_id", (int)factionId);
  auto entity = registry.create();

  registry.emplace<TransformComponent>(
      entity, sf::Vector2f(position.x * WorldConfig::WORLD_SCALE,
                           position.y * WorldConfig::WORLD_SCALE));
  auto &fMgr = FactionManager::instance();
  auto &fData = fMgr.getFaction(factionId);

  // Name based on belief (set later, default to "Vessel")
  registry.emplace<NameComponent>(entity, fData.name + " Vessel");

  Faction f;
  f.allegiances[factionId] = 1.0f;
  registry.emplace<Faction>(entity, f);

  NPCComponent npc;
  npc.factionId = factionId;
  npc.isPlayerFleet = isPlayerFleet;
  npc.leaderEntity = leaderEntity;
  npc.phaseOffset = static_cast<float>(rand() % 628) / 100.0f;

  if (isPlayerFleet) {
    npc.belief = AIBelief::Escort;
    npc.targetEntity = leaderEntity;
  }

  registry.emplace<NPCComponent>(entity, npc);

  registry.emplace<CargoComponent>(entity);
  registry.emplace<CreditsComponent>(entity, 500.0f);
  registry.emplace<ShipStats>(entity);
  registry.emplace<WeaponComponent>(entity);

  // Box2D Body creation
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = {position.x, position.y};
  bodyDef.linearDamping = npcConfig_.linearDamping;
  bodyDef.angularDamping = npcConfig_.angularDamping;

  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2Polygon dynamicBox = b2MakeBox(0.6f, 0.4f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  shapeDef.filter.maskBits = 0; // Disable physical collisions
  b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

  registry.emplace<InertialBody>(entity, bodyId, npcConfig_.thrustForce,
                                 npcConfig_.rotationSpeed,
                                 npcConfig_.maxLinearVelocity);

  // Create faction-colored sprite based on vessel type
  sf::Image img({24, 24}, sf::Color::Transparent);
  VesselType vType = npc.vesselType;

  for (int x = 0; x < 24; ++x) {
    for (int y = 0; y < 24; ++y) {
      int cx = x - 12;
      int cy = y - 12;
      bool draw = false;

      if (vType == VesselType::Military) {
        // Sharp wedge/triangle
        if (y >= 12 - x / 2 && y <= 12 + x / 2 && x <= 20)
          draw = true;
      } else if (vType == VesselType::Freight) {
        // Blocky rectangle
        if (std::abs(cx) <= 8 && std::abs(cy) <= 5)
          draw = true;
      } else {
        // Passenger: Sleek oval
        float dx = cx / 10.0f;
        float dy = cy / 6.0f;
        if (dx * dx + dy * dy <= 1.0f)
          draw = true;
      }

      if (draw) {
        img.setPixel(
            {static_cast<unsigned int>(x), static_cast<unsigned int>(y)},
            fData.color);
      }
    }
  }
  auto texture = std::make_shared<sf::Texture>();
  texture->loadFromImage(img);
  SpriteComponent sc;
  sc.texture = texture;
  sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
  sc.sprite->setOrigin({12.0f, 12.0f});
  registry.emplace<SpriteComponent>(entity, sc);

  std::cout << "[NPC] Spawned " << fData.name << " "
            << registry.get<NameComponent>(entity).name << "\n";

  span->End();
  return entity;
}

} // namespace space
