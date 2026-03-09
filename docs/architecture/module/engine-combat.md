---
id: engine-combat-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Engine Combat

# Module: Engine Combat

Ship-to-ship engagement, projectile lifecycles, and damage resolution.

## 1. Physical Scope
- **Path:** `/src/engine/combat/` — `WeaponSystem.h/.cpp`
- **Components:** `/src/game/components/WeaponComponent.h`, `ShipStats.h`, `InstalledModules.h` (`InstalledAmmo`), `ShipModule.h` (`AmmoDef`, `WeaponType`)
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Combat](/docs/architecture/capability/combat.md) (T2)

## 3. Tiered Weapon Mechanics
- **Energy (`WeaponType::Energy`)**: No ammo requirement. Draws from `batteryLevel`. Attributes: Range, Accuracy, ROF, Efficiency.
- **Projectile (`WeaponType::Projectile`)**: Ballistic (no self-acceleration). Consumes `AmmoStack` matching the weapon's `Caliber`. Ammo defines `Warhead` (Kinetic, Explosive, EMP) and `Mass/Volume`. 
- **Missile (`WeaponType::Missile`)**: Self-propelled (acceleration trait). Consumes `AmmoStack` matching `Caliber`. Ammo defines `Warhead`, `Range`, and `Guidance` (Unguided, Heat-Seeking, Fly-by-wire).

## 3. Derelict & EMP State
- **Staffing**: Vessels with manned components (Cockpit/Bridge) become derelict if unstaffed.
- **EMP**: EMP warheads incapacitate victims for 60 seconds (sets `empTimer`), ships without manned command components will not recover on their own from EMP and must be boarded to be repaired.
- **Control Block**: Derelict vessels cannot thrust, rotate, or fire weapons.

## 4. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `WeaponComponent`, `ProjectileComponent`, `InstalledAmmo`
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `WeaponSystem::update` handles ammo consumption, cooldown tracking, and EMP recovery.
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P) — `combat.weapon.fire`, `combat.collision.resolve`
- [kinematics-newtonian-2d](/docs/developer/pattern/kinematics-newtonian-2d.md) (P) — Projectile launch alignment (-Y forward)
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P)

## 4. Telemetry & Observability
- `game.combat.weapon.fire` — attributes: `combat.projectile_speed`
- `game.combat.collision.resolve` — attributes: `combat.hits`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0 → OTLP/HTTP → Jaeger
