#pragma once
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

namespace space {

enum class AIBelief { Trader, Escort, Raider };

enum class AIState { Idle, Docked, Traveling, Combat, Fleeing };

struct NPCComponent {
  uint32_t factionId;
  AIBelief belief = AIBelief::Trader;
  AIState state = AIState::Idle;

  entt::entity targetEntity = entt::null;
  sf::Vector2f targetPosition;

  float decisionTimer = 0.0f;
};

} // namespace space
