---
id: combat-capability
type: capability
pillar: architecture
---
# Capability: Combat

## 1. Business Intent
Enable ship-to-ship engagement through projectile and beam-based weapon systems. This capability manages the lifecycle of projectiles, damage calculations, and health state permanence.

## 2. Orchestration Flow
1. **Weapon Fire:** `WeaponSystem::fire` checks cooldown and energy, deducts from `ShipStats`, spawns a `ProjectileComponent` entity with a Box2D bullet body.
2. **Trajectory Calculation:** Projectiles receive initial velocity from owner orientation along the +X forward axis. Thrust and weapon firing are aligned to this same axis for intuitive control.
3. **Hit Verification:** `WeaponSystem::handleCollisions` performs distance-based proximity checks between projectiles and ships (excluding owner).
4. **Damage Resolution:** Deducts damage from `ShipStats.currentHull`; destroys projectile on hit.
5. **TTL Cleanup:** Expired projectiles are destroyed each tick.

## 3. Data Flow & Integrity
- **Trigger:** Player "Fire" action or AI combat behavior.
- **Output:** Projectile entities in the world; entity health status changes.
- **Consistency:** ACID compliance on final health state changes to prevent "ghost kills."

## 4. Operational Context
- **Primary Module:** [game-combat](/docs/architecture/module/game-combat.md) (T3)
- **Critical Failure Metric:** Hit registration latency exceeding 200ms or health desync.
