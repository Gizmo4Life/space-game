---
id: observability-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Observability

# Standard: Observability

## 1. Span Naming Convention
*Nuance: To ensure global searchability and consistent dashboarding, all telemetry spans must follow the hierarchical dot-notation.*

- **Pattern:** `<pillar>.<module>.<action>`
- **Examples:**
  - `engine.physics.step`
  - `game.combat.weapon.fire`
  - `ai.navigation.path.calculate`

## 2. Documentation Requirements
Every **Implementation** [T3 Module](/docs/developer/pattern/doc-t3-module.md) (those mapping to physical `/src`) must include a "Telemetry & Observability" section listing:
- **Probes/Spans**: The physical strings used in the code.
- **Status**: Whether the instrumentation is physical (`[x]`) or strictly theoretical (`[ ]`).
- **Nomenclature**: Spans must adhere to the `<pillar>.<module>.<action>` convention.

## 3. Critical Failure Metrics (T2)
Every [T2 Capability](/docs/developer/pattern/doc-t2-capability.md) must define at least one "Critical Failure Metric" in its Operational Context section.
- **Example:** "Transaction failure rate exceeds 2% over a 5-minute window."
