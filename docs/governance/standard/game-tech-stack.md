---
id: game-tech-stack-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](/docs/governance/standard/readme.md) > Standard: Game Tech Stack (C++)
# Standard: Game Tech Stack (C++)

Establish the technical foundations for the space-game proof-of-concept using C++.

## 1. Engine & Rendering
*Nuance: C++ provides the low-level control needed for highly optimized 2D space flight.*

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| **SFML** | **P** | High-level, developer-friendly; excellent for rapid prototyping and 2D games. |
| **SDL2** | **A** | Robust, cross-platform industry standard; more low-level than SFML. |
| **Raylib** | **A** | Simple and efficient, but lacks the maturity of SFML/SDL2 for large-scale systems. |
| [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) | **P** | Use for mapping simulation meters to screen pixels. |

## 2. Physics & Kinematics
*Nuance: Newtonian physics must be deterministic and performant.*

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| **Box2D** | **P** | The gold standard for 2D physics in C++. Highly stable and performant. |
| [kinematics-newtonian-2d](/docs/developer/pattern/kinematics-newtonian-2d.md) | **P** | Strategic target for the "Escape Velocity" flight model. |
| **Chipmunk2D** | **A** | Pivot-joint-heavy but generally less adopted than Box2D. |
| **Custom Integration** | **D** | High risk; only if Box2D cannot handle specific "Escape Velocity" orbital mechanics. |

## 3. Architecture
*Nuance: ECS is the preferred pattern for high-performance game object management in C++.*

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| **EnTT** | **P** | Modern C++ ECS with exceptional performance and a wide range of features. |
| [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) | **P** | Required shape for all game data structures. |
| [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) | **P** | Required shape for all game logic systems. |
| [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md) | **P** | Canonically unique entities (e.g., flagship) identified via shared utilities (`findFlagship`, `blueprintFromEntity`). |
| [blueprint-round-trip](/docs/developer/pattern/blueprint-round-trip.md) | **P** | `applyBlueprint` and `blueprintFromEntity` are proven inverses; verified by round-trip tests. |
| [ghost-logic](/docs/developer/pattern/ghost-logic.md) | **U** | Unacceptable. Duplicate inline lookups or component aggregation spread across multiple files. |
| **Flecs** | **A** | Excellent API and focus on usability; a strong contender to EnTT. |
| **Deep Inheritance** | **U** | Strictly forbidden due to cache-unfriendly memory layouts and fragility. |

## 4. Environment & Build
*Nuance: Standardized C++ build systems for portability.*

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| **CMake** | **P** | The de facto standard for C++ project configuration and dependency management. |
| **Make** | **D** | Less portable and harder to manage for modern multi-dependency projects. |

## 5. Economy & Factions
*Nuance: ECS-driven simulation of resource flow, faction politics, and NPC AI.*

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) | **P** | Required for `PlanetEconomy`, `CargoComponent`, `Faction`, `NPCComponent`, `CreditsComponent`. |
| **Singleton Manager** | **A** | `EconomyManager`, `FactionManager`, `NPCShipManager` use static `instance()` singletons. Acceptable for prototyping; evaluate ECS-system refactor later. |
| [trade-static-interface](/docs/developer/pattern/trade-static-interface.md) | **P** | `TradeManager` uses static buy/sell methods for atomic cargo transactions. |
| [economy-faction-standards](/docs/developer/pattern/economy-faction-standards.md) | **P** | Factions cache their best modular variants. Inferior variants go to Scrap Yards. |
| [economy-dynamic-pricing](/docs/developer/pattern/economy-dynamic-pricing.md) | **P** | Live market price calculations utilizing resource scarcity. |
| [economy-resource-chain](/docs/developer/pattern/economy-resource-chain.md) | **P** | Resource production → consumption → depletion chains. |
| [economy-competitive-market](/docs/developer/pattern/economy-competitive-market.md) | **P** | Faction bid model for tiered vessel procurement. |
| [economy-infrastructure-expansion](/docs/developer/pattern/economy-infrastructure-expansion.md) | **P** | Autonomous factory construction driven by faction budget. |
| [economy-market-structure](/docs/developer/pattern/economy-market-structure.md) | **P** | Physical ship inventory and market listing structure. |
| [economy-refit-fee](/docs/developer/pattern/economy-refit-fee.md) | **A** | Installation credit costs for player refitting. |
| [tiered-utility-allocation](/docs/developer/pattern/tiered-utility-allocation.md) | **P** | Differentiating item attributes based on technology tiers. |
| [faction-dna-genetic-profile](/docs/developer/pattern/faction-dna-genetic-profile.md) | **P** | Strategic axis weights for procedural behavior. |
| [faction-relationship-matrix](/docs/developer/pattern/faction-relationship-matrix.md) | **P** | N×N relationship matrix governing faction diplomacy. |
| [npc-ai-state-machine](/docs/developer/pattern/npc-ai-state-machine.md) | **P** | Finite state machine for NPC behavioral orchestration. |
| [npc-fleet-leader-boids](/docs/developer/pattern/npc-fleet-leader-boids.md) | **A** | Boid-based wingman formation following fleet leader. |

## 6. Combat & Logistics
*Nuance: Tiered combat requires strict resource discipline and unified slot definitions.*

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| **Tiered Ammo Matrix** | **P** | 12-type matrix (T2/T3) managed via `AmmoMagazine` volume logic. |
| [unified-slot-system](/docs/developer/pattern/unified-slot-system.md) | **P** | Positional role assignment in `HullDef` (Command/Hardpoint/Engine). |
| [boarding-protocol](/docs/developer/pattern/boarding-protocol.md) | **P** | Ship-to-ship resource transfer and fleet joining for derelict/friendly targets. |
| [resource-depletion-cascade](/docs/developer/pattern/resource-depletion-cascade.md) | **P** | Tiered consequence chain on resource exhaustion (starvation → power failure → drift). |
| **Wet Mass Dynamics** | **P** | Kinematics must account for cargo, fuel, and ammo mass in physics steps. |
| **Incapacitation (EMP)** | **P** | EMP warheads trigger `empTimer` (60s). Incapacitated vessels are derelict. |

## 7. Observability & Dashboards
*Nuance: Deep observability is the primary tool for debugging complex multi-agent simulations.*

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| **SigNoz** | **P** | Unified OTel-native platform for traces, metrics, and logs. Preferred for dashboards. |
| **Jaeger** | **A** | Lightweight local tracing; useful for deep dive into specific spans. |
| **OTel Collector** | **P** | Fan-out component ensuring telemetry reaches multiple backends. |
| [doc-ops-diagnostic-dashboard](/docs/developer/pattern/doc-ops-diagnostic-dashboard.md) | **P** | Standard manifest for defining troubleshooting views. |
| [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) | **P** | Use for logic-heavy operations (e.g., `reequipForDuration`). |

## 8. Simulation Integrity
*Nuance: High-frequency simulation logic must remain stable regardless of frame rate or execution density.*

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| [rate-based-consumption-scaling](/docs/developer/pattern/rate-based-consumption-scaling.md) | **P** | Required. All resource draw (fuel/isotopes/food) must be scaled by `deltaTime`. |
| [greedy-fitness-generation-retry](/docs/developer/pattern/greedy-fitness-generation-retry.md) | **P** | Preferred. Use for procedural generation where a minimum quality floor is required. |
| [deterministic-attribute-tiering](/docs/developer/pattern/deterministic-attribute-tiering.md) | **P** | Discrete integer-based tier scaling over continuous random variables. |
| [attribute-differentiated-recipes](/docs/developer/pattern/attribute-differentiated-recipes.md) | **P** | Scaling production components logically based on quality/tiers. |
| [dna-weighted-infrastructure-expansion](/docs/developer/pattern/dna-weighted-infrastructure-expansion.md) | **P** | Using faction DNA priorities (e.g., `tradePriority`) to bias autonomous building. |
| [evolutionary-strategy-drift](/docs/developer/pattern/evolutionary-strategy-drift.md) | **P** | Stochastic DNA mutation based on performance metrics. |
| [procedural-hull-generation](/docs/developer/pattern/procedural-hull-generation.md) | **P** | Role-based construction of vessel geometries. |
| [mission-performance-feedback-loop](/docs/developer/pattern/mission-performance-feedback-loop.md) | **P** | K/D tracking drives generational evolution. |
| [world-procedural-generation](/docs/developer/pattern/world-procedural-generation.md) | **A** | Procedural star system and planet generation. |
| [frame-fixed-draw](#) | **U** | Unacceptable. Resource consumption that occurs at a fixed rate per frame. |
