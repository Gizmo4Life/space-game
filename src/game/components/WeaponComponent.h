#pragma once
#include "game/components/AmmoComponent.h"
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

  float mass = 10.0f;
  float volume = 5.0f;
};

} // namespace space
