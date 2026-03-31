---
id: span-world-gen-anomaly
type: span_runbook
module: game-core-module
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > world.gen.anomaly

# Span Runbook: World Generation Anomaly

## 1. Component Scope
- **Module:** [World Loader](/docs/architecture/module/game-core.md)
- **Signal:** Failed spans or extreme latency in `game.world.gen.*` spans.
- **Description:** Tracks failures during procedural system generation, including empty orbits or unpopulated economies.

## 2. Diagnostic Mapping
| Metric | Threshold | logic |
| :--- | :--- | :--- |
| **Star Count** | < 1 | System barycenter exists but no stars generated. |
| **Body Count** | < 3 | Inner/Outer orbits failed to populate celestial bodies. |
| **Economy Score** | 0 station units | Planets exist but `game.world.gen.economy_seed` failed to assign factions. |

**Isolate Failure:**
- Check `game.world.gen.orbital_system` for `parent` entity validity.
- Verify `rand()` seeding in `WorldLoader.cpp`; if deterministic and broken, check seed source.
- Inspect `StarCount` and `BodyCount` attributes in SigNoz.

## 3. Mitigation & Restoration
1. **Re-seed Universe:** Restart the game instance to trigger a new `WorldLoader::generateStarSystem` call with a fresh seed.
2. **Manual Planet Injection:** If a critical Earthlike planet is missing, use dev-console to trigger `WorldLoader::seedEconomy` on a Rocky body.
3. **Fix Faction Table:** If `economy_seed` is failing, verify `FactionManager::getAllFactions` is not empty.

## 4. Recovery Verification
- `game.core.world.load` span completes within 500ms.
- Parent-Child hierarchy shows: `StarSystem` -> `OrbitalSystem` -> `CelestialBody` (Min 6).
