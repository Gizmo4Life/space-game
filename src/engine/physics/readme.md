# /src/engine/physics/

→ [T3 Module: Physics](/docs/architecture/module/physics.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)

## Systems
- `PhysicsEngine` — Owns the Box2D `b2WorldId`, steps the simulation each frame
- `KinematicsSystem` — Applies thrust/rotation forces, syncs Box2D positions to `TransformComponent`
- `GravitySystem` — Applies gravitational pull from `CelestialBody` to `InertialBody` entities
- `OrbitalSystem` — Updates planet/moon positions via Kepler ellipse equations
- `AsteroidSystem` — Asteroid field spawning and management

## Build
```bash
cmake --build build --target SpaceGame
```
