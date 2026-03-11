---
id: span-world-gen-anomaly
3: type: span_runbook
4: module: game-core-module
5: pillar: operational
6: ---
7: [Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > world.gen.anomaly
8: 
9: # Span Runbook: World Generation Anomaly
10: 
11: ## 1. Component Scope
12: - **Module:** [World Loader](/docs/architecture/module/game-core.md)
13: - **Signal:** Failed spans or extreme latency in `game.world.gen.*` spans.
14: - **Description:** Tracks failures during procedural system generation, including empty orbits or unpopulated economies.
15: 
16: ## 2. Diagnostic Mapping
17: | Metric | Threshold | logic |
18: | :--- | :--- | :--- |
19: | **Star Count** | < 1 | System barycenter exists but no stars generated. |
20: | **Body Count** | < 3 | Inner/Outer orbits failed to populate celestial bodies. |
21: | **Economy Score** | 0 station units | Planets exist but `game.world.gen.economy_seed` failed to assign factions. |
22: 
23: **Isolate Failure:**
24: - Check `game.world.gen.orbital_system` for `parent` entity validity.
25: - Verify `rand()` seeding in `WorldLoader.cpp`; if deterministic and broken, check seed source.
26: - Inspect `StarCount` and `BodyCount` attributes in SigNoz.
27: 
28: ## 3. Mitigation & Restoration
29: 1. **Re-seed Universe:** Restart the game instance to trigger a new `WorldLoader::generateStarSystem` call with a fresh seed.
30: 2. **Manual Planet Injection:** If a critical Earthlike planet is missing, use dev-console to trigger `WorldLoader::seedEconomy` on a Rocky body.
31: 3. **Fix Faction Table:** If `economy_seed` is failing, verify `FactionManager::getAllFactions` is not empty.
32: 
33: ## 4. Recovery Verification
34: - `game.core.world.load` span completes within 500ms.
35: - Parent-Child hierarchy shows: `StarSystem` -> `OrbitalSystem` -> `CelestialBody` (Min 6).
