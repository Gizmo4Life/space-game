---
id: architecture-module-manifest
type: manifest
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > Module

# Sub-pillar: Module

Physical code clusters and implementation mappings.

```mermaid
graph TD
    subgraph System Core
        Core[src-core]
        Sys[doc-system]
        AI[ai-config]
    end
    subgraph Governance Layer
        Prot[governance-protocols]
        Stan[governance-standards]
        Pat[developer-patterns]
    end
    subgraph Operational Edge
        Work[agent-workflows]
        Ext[external-pillar]
        Ops[operational-pillar]
    end

    Sys --> Core
    Pat --> Stan
    Stan --> Prot
    Prot --> Work
    Work --> Ops
    Ext --> Core
```

## 1. System Core
*Nuance: The primary implementation logic for the RAG engine.*
- [src-core](src-core.md): Physical application logic.
- [doc-system](doc-system.md): File system structure and DaC engine.
- [ai-config](ai-config.md): Persona behavioral injections.

## 2. Game Engine (C++)
*Nuance: High-performance modules implementing the spatial simulation and interactive systems.*
- [physics](physics.md): Box2D-powered Newtonian physics engine.
- [system-gate](system-gate.md): Inter-system jump and loading logic.
- [game-combat](game-combat.md): Ship-to-ship engagement and projectile systems.
- [game-economy](game-economy.md): Faction budgeting, trade, and NPC orchestration.

## 2. Governance Layer
*Nuance: The "Brain" of the repository, defining the rules and protocols.*
- [developer-patterns](developer-patterns.md): Atomic structural definitions.
- [governance-standards](governance-standards.md): Compliance and fitness rules.
- [governance-protocols](governance-protocols.md): Idempotent execution sequences.

## 3. Operational Edge
*Nuance: Automation and response logic for external boundaries and runtime.*
- [agent-workflows](agent-workflows.md): Protocol automation.
- [external-pillar](external-pillar.md): System boundaries and adoption.
- [operational-pillar](operational-pillar.md): Restoration and triage logic.
