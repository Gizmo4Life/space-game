#pragma once
#include "game/components/Economy.h"
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

namespace space {

enum class AIBelief { Trader, Escort, Raider };

enum class AIState { Idle, Docked, Traveling, Combat, Fleeing };

struct NPCComponent {
  uint32_t factionId;
  VesselType vesselType = VesselType::Passenger;
  AIBelief belief = AIBelief::Trader;
  AIState state = AIState::Idle;
  bool isPlayerFleet = false;
  entt::entity leaderEntity = entt::null;

  entt::entity targetEntity = entt::null;
  entt::entity homePlanet = entt::null;
  sf::Vector2f targetPosition;

  float decisionTimer = 0.0f;
  float dockTimer = 0.0f;      // Time remaining docked
  float arrivalRadius = 30.0f; // How close counts as "arrived"
  float patrolAngle = 0.0f;    // For escort circular patrol
  float phaseOffset = 0.0f;    // Unique offset to prevent clustering
  float lifeTimer = 0.0f;      // Persistent clock for dynamic patterns

  // Orchestration
  float passengerCount = 0.0f;                // Population in transit
  Resource constructionTarget = (Resource)-1; // Factory type to build
};

} // namespace space
