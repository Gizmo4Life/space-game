---
id: rendering-dual-scale
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Rendering Dual Scale

# Pattern: Rendering Dual Scale

**Intent:** Run two physically distinct coordinate scales simultaneously — one for solar-system-level distances and orbital mechanics, one for foreground ship combat — so that ships appear at a plausible screen size without compressing orbital distances.

## Shape

```cpp
// WorldConfig.h
constexpr float WORLD_SCALE = 0.05f;  // metres→pixels for orbital bodies
constexpr float SHIP_SCALE  = 30.0f;  // metres→pixels for ships & combat

// Usage
sf::Vector2f orbitalPos = transform.worldPos * WORLD_SCALE;  // background pass
sf::Vector2f shipPos    = body.position      * SHIP_SCALE;   // foreground pass
```

## Key Constraints
- **Two contexts, never mixed:** Each render pass selects exactly one scale constant; mixing them produces incorrect positions.
- **Both constants live in `WorldConfig.h`:** Single source of truth; never hardcoded at call sites.
- **Background pass** uses `WORLD_SCALE` — planets, stars, orbital paths.
- **Foreground pass** uses `SHIP_SCALE` — ships, projectiles, physics bodies.
- **UI pass** uses neither — drawn in screen-space via `window.setView(defaultView)`.

## Applied In
- `RenderSystem::update` — Background (orbital) vs. foreground (physics body) render separation.
- `GravitySystem` / `OrbitalSystem` — Orbital calculations use world-space coordinates directly.
- `KinematicsSystem` — Thrust applied in Box2D world-space; `SHIP_SCALE` applied at render only.
- Extended by [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) — Box2D→SFML coordinate transform.
