---
id: full-dependency-initialization
type: pattern
pillar: developer
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Full Dependency Initialization (for Tests)

# Pattern: Full Dependency Initialization (for Tests)

## Context
Complex game systems often have "early escape" guards to prevent crashes when optional or mandatory components are missing. In unit tests, if you only initialize a subset of components, you might inadvertently trigger one of these guards, causing the system to skip the logic you actually intended to test. This can lead to "false pass" scenarios.

## Preferred (P)
**Initialize all mandatory and relevant context components required for the system's logic to execute fully.** Refer to the system's "guard" section to identify these dependencies.

```cpp
// Preferred: Full context initialization
auto entity = registry.create();
registry.emplace<InertialBody>(entity, bodyId);
registry.emplace<ShipStats>(entity);
registry.emplace<HullDef>(entity); // Mandatory for Kinematics calculation
registry.emplace<TransformComponent>(entity);

// Now update will actually process the logic
KinematicsSystem::update(registry, 0.01f);
```

## Unacceptable (U)
**Partial dependency setup that relies on silent logic bypasses.** Testing a system without its required context components.

```cpp
// Unacceptable: Missing HullDef causes KinematicsSystem to return early
auto entity = registry.create();
registry.emplace<InertialBody>(entity, bodyId);

KinematicsSystem::update(registry, 0.01f); 
// Test might "pass" assertions because nothing crashed, 
// but NO physics logic was actually executed!
```

## Nuance
If you are specifically testing the guard logic itself (e.g., ensuring the system *does* return early without a hull), then partial setup is acceptable, but it must be explicitly stated in the test name (e.g., `TEST_CASE("KinematicsSystem Early Return Without Hull")`).
