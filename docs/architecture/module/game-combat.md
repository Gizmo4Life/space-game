---
id: game-combat-module
type: module
pillar: architecture
dependencies: ["physics-module"]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Combat

# Module: Game Combat

Ship-to-ship engagement, projectile lifecycles, and damage resolution.

## 1. Physical Scope
- **Path:** `/src/engine/combat/`
- **Components:** `/src/game/components/WeaponComponent.h`, `ShipStats.h`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability] Combat (T2)

## 3. Key Systems
- **WeaponSystem**: Manages cooldowns, projectile spawning (Box2D bullet bodies), and distance-based collision detection.
- **Firing Sequence**: Check constraints → Spend energy → Spawn projectile → Set velocity from owner orientation.
- **handleCollisions**: Iterates projectiles vs. ships (excluding owner), applies hull damage on proximity hit.

## 4. Pattern Composition
- [Pattern] cpp-ecs-component (P) — `WeaponComponent`, `ProjectileComponent`, `ShipStats`
- [Pattern] cpp-ecs-system-static (P) — `WeaponSystem::update`, `WeaponSystem::fire`
- [Pattern] logic-idempotency (P)

## 5. Telemetry & Observability
- **Semantic Spans:** `combat.weapon.fire`, `combat.collision.resolve`
- **Health Probes:** `combat.projectile.active_count`, `combat.damage.total_inflicted`
- **Status:** ⚠️ _Declared but not yet instrumented in source code._
