---
id: cpp-ecs-system-static
type: pattern
tags: [cpp, ecs, logic, stateless]
---
# Pattern: C++ ECS System (Static)

## 1. Geometry
- **Requirement:** Implement logic as a `class` or `namespace` with `static` methods.
- **Requirement:** Accept an ECS registry reference and a delta-time float as primary inputs.
- **Rule:** Systems must be "stateless" at the class level; all state must reside within [Components].
- **Rule:** Define update methods that iterate over specific [Component] views.

## 2. Nuance
Using static methods enforces a clear separation between data (Components) and logic (Systems), facilitating easier testing and minimizing side effects.

## 3. Verify
- The class has no member variables.
- Methods are either `static` or `const`.
- Logic is focused strictly on transforming Component state.
