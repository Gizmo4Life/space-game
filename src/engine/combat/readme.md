# /src/engine/combat/

→ [Home](/docs/readme.md)
→ [T3 Module: Engine Combat](/docs/architecture/module/engine-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)
→ [Standard: Observability](/docs/governance/standard/observability-standard.md)

## Systems
- `WeaponSystem` — Manages procedural firing logic (Energy Beams, Ballistic Projectiles, Guided Missiles) and attribute scaling.
- `CollisionSystem` (shared) — Handles physics-based kinetic damage and Area-of-Effect (AOE) explosion triggers.

## Coding Standards
- **Weapon Archetypes**: All new weapon logic MUST comply with the [Weapon Archetype Standard](/docs/governance/standard/weapon-archetype-standard.md).
- **Hit Detection**: 
    - **Energy**: Use Box2D Raycasting ($b2World\_CastRayClosest$) for instant hits.
    - **Ballistic/Missile**: Use Box2D Collision Events ($b2ContactEvents$) for impact resolution. Damage is physics-calculated ($0.5mv^2$).
- **Spans**: Every weapon fire event should emit an OTel span with calculated attributes (mass, acceleration, range).

## Build
```bash
cmake --build build --target SpaceGame
```
