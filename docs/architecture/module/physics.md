---
id: physics-module
type: module
pillar: architecture
dependencies: []
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Physics

# Module: Physics

Newtonian kinematics, gravity wells, orbital mechanics, and Box2D world management.

## 1. Physical Scope
- **Path:** `/src/engine/physics/`
- **Systems:** `KinematicsSystem`, `GravitySystem`, `OrbitalSystem`, `PhysicsEngine`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability] Navigation (T2)
- [Capability] Combat (T2)

## 3. Key Systems
- **PhysicsEngine**: Owns the Box2D `b2WorldId`, steps the simulation each frame.
- **KinematicsSystem**: Applies thrust/rotation forces and syncs Box2D positions with `TransformComponent`.
- **GravitySystem**: Iterates `CelestialBody` × `InertialBody` pairs, applies gravitational pull.
- **OrbitalSystem**: Updates `TransformComponent` via Kepler ellipse equations for moons/planets.

## 4. Pattern Composition
- [Pattern] cpp-ecs-system-static (P) — `KinematicsSystem`, `GravitySystem`, `OrbitalSystem`
- [Pattern] cpp-ecs-component (P) — `InertialBody`, `OrbitalComponent`, `TransformComponent`
- [Pattern] kinematics-newtonian-2d (P) — Gravity and thrust calculations
- [Pattern] cpp-singleton-manager (P) — `PhysicsEngine`
- [Pattern] logic-idempotency (P)

## 5. Telemetry & Observability
- **Status:** 🔲 Not yet instrumented — candidate spans: `physics.step`, `gravity.update`, `orbital.update`
