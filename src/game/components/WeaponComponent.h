#pragma once
#include "GameTypes.h"
#include "game/components/AmmoComponent.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

namespace space {

enum class WeaponTier { T1_Energy, T2_Projectile, T3_Missile };
enum class WeaponMode { Safety, Active };

struct ProjectileComponent {
  float damage = 10.0f;
  float timeToLive = 3.0f;
  entt::entity owner = entt::null;
  bool isEmp = false;
  bool isExplosive = false;
  float mass = 1.0f;        // Applied to impulse on impact
  float visualScale = 1.0f; // Multiplier for sprite size
};

struct BeamComponent {
  sf::Vector2f origin;
  sf::Vector2f direction;
  float length = 0.0f;
  float duration = 0.2f;          // Remaining time to render
  float maxDuration = 0.2f;       // Total lifespan for fading
  float damagePerTick = 10.0f;
  entt::entity owner = entt::null;
  float width = 1.0f;
  sf::Color color = sf::Color::White;
};

struct ExplosionComponent {
  sf::Vector2f position;
  float radius = 20.0f;
  float damage = 50.0f;
  bool isEmp = false;
  float duration = 0.5f;
  float maxDuration = 0.5f;
  entt::entity owner = entt::null;
};

struct MissileComponent {
  float acceleration = 100.0f;
  float maxSpeed = 5000.0f;
  float turnRate = 5.0f;
  entt::entity target = entt::null;
  bool isHeatSeeking = false;
  bool isRemote = false;
};

struct WeaponComponent {
  WeaponTier tier = WeaponTier::T1_Energy;
  WeaponMode mode = WeaponMode::Safety;

  float fireCooldown = 0.5f;
  float currentCooldown = 0.0f;
  float projectileSpeed = 5000.0f;
  float energyCost = 5.0f;
  float baseDamage = 10.0f;

  AmmoType selectedAmmo;
  
  // Tiers for formula-based scaling
  Tier rangeTier = Tier::T1;
  Tier caliberTier = Tier::T1;
  Tier rofTier = Tier::T1;
  Tier efficiencyTier = Tier::T1;
  float qualityRoll = 1.0f;

  float mass = 10.0f;
  float volume = 5.0f;
};

} // namespace space
