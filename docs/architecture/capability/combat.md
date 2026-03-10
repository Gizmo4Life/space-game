---
id: combat-capability
type: capability
pillar: architecture
---
[Home](/) > [Architecture](/docs/architecture/readme.md) > [Capability](readme.md) > Combat

# Capability: Combat

## 1. Business Intent
Enable multi-layered ship-to-ship engagement through distinct weapon subtypes (Energy, Projectile, Missile). This capability manages the lifecycle of projectiles, dynamic ammunition consumption (`AmmoDef`), damage calculations from tiered warheads, and health state permanence.

## 2. Orchestration Flow
1. **Weapon Fire:** `WeaponSystem::fire` checks cooldowns and deducts resources. Energy weapons draw from `ShipStats` battery; Projectiles and Missiles consume specific matching `AmmoStack` inventory from `InstalledAmmo`. Spawns a `ProjectileComponent` entity with a Box2D bullet body.
2. **Trajectory Calculation:** Projectiles receive initial velocity from owner orientation along the +X forward axis. Thrust and weapon firing are aligned to this same axis for intuitive control.
3. **Hit Verification:** `WeaponSystem::handleCollisions` performs distance-based proximity checks between projectiles and ships (excluding owner). Missiles optionally utilize `AttributeType::Guidance` for mid-flight targeting adjustments.
4. **Damage Resolution:** Deducts damage from `ShipStats.currentHull` based on weapon damage and Ammo payload (`Tier::Kinetic/Explosive/EMP`). Destroys projectile on hit.
5. **TTL Cleanup:** Expired projectiles (or those reaching `Range` limit) are destroyed each tick.

## 3. Data Flow & Integrity
- **Trigger:** Player "Fire" action or AI combat behavior.
- **Output:** Projectile entities in the world; entity health status changes.
- **Consistency:** ACID compliance on final health state changes to prevent "ghost kills."

## 4. Verification Protocol
- **Automated Tests:** `test_weapons.cpp` verifies projectile spawning, resource costs, and missile guidance logic.
- **Physics Validation:** `test_collisions.cpp` ensures physical impact damage between ships is calculated and attributed correctly.
- **Regression Check:** Focus on `WeaponMode::Active` state and `enableContactEvents=true` in shape definitions to prevent silent collision failure.

## 5. Operational Context
- **Primary Module:** [engine-combat](/docs/architecture/module/engine-combat.md) (T3)
- **Critical Failure Metric:** Hit registration latency exceeding 200ms or health desync.
