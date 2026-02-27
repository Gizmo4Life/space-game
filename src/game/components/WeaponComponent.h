#pragma once
#include <entt/entt.hpp>

namespace space {

struct ProjectileComponent {
  float damage = 10.0f;
  float timeToLive = 3.0f; // Seconds before destruction
  entt::entity owner = entt::null;
};

struct WeaponComponent {
  float fireCooldown = 0.5f;
  float currentCooldown = 0.0f;
  float projectileSpeed = 1500.0f;
  float energyCost = 5.0f;
};

} // namespace space
