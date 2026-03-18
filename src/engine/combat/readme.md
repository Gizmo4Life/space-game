# /src/engine/combat/

→ [Home](/docs/readme.md)
→ [T3 Module: Engine Combat](/docs/architecture/module/engine-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)
→ [Standard: Observability](/docs/governance/standard/observability-standard.md)

## Systems
- `WeaponSystem` — Projectile spawning, cooldown management, distance-based collision and hull damage.

## Coding Standards
- **Ammo Access**: Ammo types must always be accessed through the `AmmoMagazine` volume logic — no direct map access that bypasses the weight accounting.
- **Hit Detection**: Use the existing distance-based collision path — do not add Box2D contact listeners for projectiles without a design review.
- **Spans**: Every weapon fire event should emit an OTel span. See [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md).

## Build
```bash
cmake --build build --target SpaceGame
```
