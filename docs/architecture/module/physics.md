---
id: physics-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Physics

# Module: Physics

Newtonian kinematics, gravity wells, orbital mechanics, and Box2D world management.

## 1. Physical Scope
- **Path:** `/src/engine/physics/`
- **Systems:** `KinematicsSystem`, `GravitySystem`, `OrbitalSystem`, `PhysicsEngine`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Navigation](/docs/architecture/capability/navigation.md) (T2)
- [Capability: Combat](/docs/architecture/capability/combat.md) (T2)

## 3. Key Systems
- **PhysicsEngine**: Owns the Box2D `b2WorldId`, steps the simulation each frame.
- **KinematicsSystem**: Applies thrust/rotation forces and syncs Box2D positions with `TransformComponent`.
- **GravitySystem**: Iterates `CelestialBody` × `InertialBody` pairs, applies gravitational pull.
- **OrbitalSystem**: Updates `TransformComponent` via Kepler ellipse equations for moons/planets.
- **Dual-Scale System:** Two named coordinate scales in `WorldConfig.h`. See [rendering-dual-scale](/docs/developer/pattern/rendering-dual-scale.md) (P).

## 4. Pattern Composition
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `KinematicsSystem`, `GravitySystem`, `OrbitalSystem`
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `InertialBody`, `OrbitalComponent`, `TransformComponent`
- [kinematics-newtonian-2d](/docs/developer/pattern/kinematics-newtonian-2d.md) (P) — Gravity and thrust calculations
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `PhysicsEngine`
- [rendering-dual-scale](/docs/developer/pattern/rendering-dual-scale.md) (P) — `WORLD_SCALE` / `SHIP_SCALE` coordinate contexts
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P)

## 5. Telemetry & Observability
- **Status:** 🔲 Not yet instrumented — candidate spans: `engine.physics.step`, `engine.physics.gravity.update`, `engine.physics.orbital.update`
