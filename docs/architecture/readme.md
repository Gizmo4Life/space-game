---
id: architecture-landscape
type: landscape
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > Architecture

# Pillar: Architecture

Global boundaries and primary domain of the AI RAG documentation system.

## 1. System Scope
*Nuance: This system governs the relationship between atomic documentation patterns and physical code implementation.* It ensures RAG-optimized knowledge retrieval for autonomous agents.

## 2. System Context
- [space-game-context](space-game-context.md): The original problem statement, premise, and success criteria for the Top-Down Space Faring engine.

## 2. Core Capabilities (T2)
*Nuance: Bounded contexts that orchestrate multiple T3 modules to deliver business value.*
- **Doc Ingestion:** Physical processing and validation of the documentation graph. *(Planned)*
- [Game Navigation](capability/navigation.md): Orchestrates ship movement and spatial transitions using Newtonian physics.
- [Game Combat](capability/combat.md): Orchestrates ship-to-ship engagement and projectile lifecycles.
- [Game Economy](capability/economy.md): Orchestrates resource flow, faction budgets, trade, and NPC AI.
- [Observability](capability/observability.md): Orchestrates telemetry instrumentation, tracing, and metrics.
- [UI Framework](capability/ui-framework.md): Orchestrates overlay management, HUD lifecycle, and screen state.
- [Governance Enforcement](capability/governance-enforcement.md): Automating protocol compliance and landscape audits.
- [Repository Consumption](capability/repository-consumption.md): Strategies for Greenfield bootstrapping and Brownfield discovery.
- [Operational Reliability](capability/operational-reliability.md): Orchestrating incident response and restoration.

## 3. External Boundaries
*Nuance: Interaction points with 3rd party services and vendor dependencies.*
- [Gemini API](https://ai.google.dev/): Primary reasoning engine for documentation audits.

## 4. Machine Navigation Metadata
```yaml
type: directory_manifest
pillar: architecture
index_map:
  capability:
    path: capability/
    scope: Business logic flows and orchestration.
  module:
    path: module/
    scope: Physical code clusters and telemetry.
  patterns:
    path: patterns/
    scope: Specific implementation and verification recipes.
  standard:
    path: standard/
    scope: Governance rules and operational standards.
```
