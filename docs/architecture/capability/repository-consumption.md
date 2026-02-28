---
id: repository-consumption
type: capability
pillar: architecture
---
[Home](/) > [Architecture](/docs/architecture/readme.md) > [Capability](readme.md) > Repository Consumption

# Capability: Repository Consumption

## 1. Business Intent
Govern how the repository is forked, merged, or integrated into external projects while keeping the RAG-optimized documentation layer intact and structurally valid in the target environment.

## 2. Orchestration Flow
1. **Context Selection:** Determine whether the target project is Greenfield (new) or Brownfield (existing) to select the appropriate adoption strategy.
2. **Baseline Verification:** Confirm the inherited code and doc structure match the source quality gates.
3. **Architectural Mapping:** Establish or import the Knowledge Graph — YAML frontmatter, breadcrumbs, and T2/T3 links — in the target project.

## 3. Data Flow & Integrity
- **Trigger:** Engineer initiating a fork, merge, or adoption of the repository.
- **Output:** Target project with a valid, navigable Knowledge Graph.
- **Consistency:** Adoption protocols are idempotent; re-running verification steps after partial adoption is safe.

## 4. Operational Context
- **Primary Module:** [external-pillar](/docs/architecture/module/external-pillar.md) (T3)
- **Critical Failure Metric:** Dead links or missing frontmatter in the target Knowledge Graph after adoption.
