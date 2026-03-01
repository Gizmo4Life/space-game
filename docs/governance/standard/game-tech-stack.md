---
id: game-tech-stack-standard
type: standard
pillar: governance
---
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
