# Engine

→ [T3 Module: Physics](/docs/architecture/module/physics.md)
→ [T3 Module: Game Combat](/docs/architecture/module/game-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)

## Subsystems
- `physics/` — Box2D integration: kinematics, gravity, orbital, asteroid systems
- `combat/` — WeaponSystem: projectile spawning and hit detection
- `telemetry/` — OpenTelemetry SDK lifecycle and span emission

## Build
```bash
cmake --build build --target SpaceGame
```
