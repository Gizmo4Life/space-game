# /src/engine/physics/

→ [T3 Module: Physics](/docs/architecture/module/engine-physics.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)
→ [Standard: Observability](/docs/governance/standard/observability-standard.md)

## Systems
- `PhysicsEngine` — Owns the Box2D `b2WorldId`, steps the simulation each frame.
- `KinematicsSystem` — Applies thrust/rotation forces, syncs Box2D positions to `TransformComponent`.
- `GravitySystem` — Applies gravitational pull from `CelestialBody` to `InertialBody` entities.
- `OrbitalSystem` — Updates planet/moon positions via Kepler ellipse equations.
- `AsteroidSystem` — Asteroid field spawning and management.

## Coding Standards
- **Wet Mass**: Kinematics must account for fuel, cargo, and ammo mass each step — do not use `dryMass` alone.
- **Force Application**: All thrust forces must go through Box2D APIs (`b2Body_ApplyForceToCenter`), not direct `TransformComponent` mutation.
- **IWYU**: Keep Box2D headers out of non-physics files. See [header-management](/docs/governance/standard/header-management.md).

## Build
```bash
cmake --build build --target SpaceGame
```
