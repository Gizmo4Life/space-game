---
id: cpp-ecs-component
type: pattern
tags: [cpp, ecs, memory, pod]
---
# Pattern: C++ ECS Component

## 1. Geometry
- **Requirement:** Define data as a Plain Old Data (POD) `struct`.
- **Requirement:** Exclude all methods, constructors, or logic.
- **Rule:** Use composition over inheritance. Components must be atomic and focused on a single responsibility.
- **Rule:** Avoid storing raw pointers; use IDs or managed handles if cross-referencing is required.

## 2. Nuance
This shape ensures optimal memory locality and cache-friendliness, as ECS registries can pack these structs into contiguous arrays.

## 3. Verify
- The struct size is minimal.
- There are no virtual methods or private members.
