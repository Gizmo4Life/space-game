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
- **Path:** `/src/engine/combat/` — `WeaponSystem.h/.cpp`
- **Components:** `/src/game/components/WeaponComponent.h`, `ShipStats.h`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Combat](/docs/architecture/capability/combat.md) (T2)

## 3. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `WeaponComponent`, `ProjectileComponent`, `ShipStats`
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `WeaponSystem::update`, `WeaponSystem::fire`
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P) — `combat.weapon.fire`, `combat.collision.resolve`
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P)

## 4. Telemetry & Observability
- `combat.weapon.fire` — attributes: `combat.projectile_speed`
- `combat.collision.resolve` — attributes: `combat.hits`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0 → OTLP/HTTP → Jaeger
