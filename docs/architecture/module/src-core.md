---
id: src-core-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Source Core

# Module: Source Core

Low-level primitives, shared utilities, and foundational types used across the engine and game.

## 1. Physical Scope
- **Path:** `/src/core/`
- **Systems:** Shared types, math utilities, and primitive definitions.
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Repository Consumption](/docs/architecture/capability/repository-consumption.md) (T2)

## 3. Description
This module serves as the foundational layer of the source tree. It contains headers and utilities that are too general for a specific engine subsystem but are required by multiple modules.

## 4. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — Foundations for data structures.
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P) — Applied to core utilities.
