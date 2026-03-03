---
id: operational-stability-standard
type: standard
tags: [governance, build, stability]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Standard](readme.md) > Operational Stability Standard

## 1. Context
High-frequency gameplay iteration using a shared EnTT-based ECS across UI, physics, and gameplay modules.

## 2. Nuance
**Compilation Success > Code Elegance > Header Minimization.**
We prioritize a codebase that compiles correctly across all environments and agents, even if it means more explicit headers or slightly more verbose namespace prefixes.

## 3. The Matrix
| Pattern | Rating | Reasoning |
| :--- | :--- | :--- |
| [cpp-header-hygiene](../pattern/cpp-header-hygiene.md) | D | Essential for preventing transitive include breaks during refactors. |
| [cpp-explicit-namespace-resolution](../pattern/cpp-explicit-namespace-resolution.md) | D | Resolves frequent EnTT/local name conflicts. |
| [cpp-component-aggregation](../pattern/cpp-component-aggregation.md) | A | Critical for keeping UI panels in sync with gameplay logic. |
| [cpp-header-resilience](../pattern/cpp-header-resilience.md) | D | Standardized protocol to prevent "Operation not permitted" and missing include errors. |

## 4. Stability Gate
All changes MUST pass a local build verification before being merged or considered "done" by an agent.
- **Pre-flight Build**: `cd /tmp/space-game-build && make -j4`
- **Lint Verification**: All "Missing Header" or "Unknown Type" errors must be resolved by explicit inclusion, following the [cpp-header-hygiene](../pattern/cpp-header-hygiene.md) pattern.
- **Namespace Resolution**: EnTT and SFML types must use explicit prefixes (`::entt::`, `::sf::`) to prevent collision regressions, as per [cpp-explicit-namespace-resolution](../pattern/cpp-explicit-namespace-resolution.md).
