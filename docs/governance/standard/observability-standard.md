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

## 4. Dashboard Requirements
Every **P-rated** [T2 Capability](/docs/architecture/capability/readme.md) must have at least one **Diagnostic Dashboard** defined in the [SigNoz Console](http://localhost:3301).
- **Mandatory Panels**:
  - **Latency (P99)** by span action.
  - **Error Rate** by module.
  - **Throughput** (Ops/sec) for critical loops (Physics, Economy).
- **Enforcement**: New features impacting a P-rated capability must update the corresponding [Diagnostic Dashboard] manifest.
- **Organization**: At current project scale, a unified "Single Pane of Glass" dashboard is preferred over multiple domain-specific dashboards to reduce operational overhead.

## 5. Span Runbook Requirements
Every critical system span (those linked to P-rated capabilities or complex state machines) **MUST** have a corresponding [Span Runbook](/docs/operational/span/readme.md).
- **Format**: Must follow the [doc-ops-span-runbook](/docs/developer/pattern/doc-ops-span-runbook.md) pattern.
- **Content**: Must include Diagnostic Mapping (Metrics -> Logic) and step-by-step Mitigation.
- **Linkage**: Runbooks must be linked from the corresponding [Diagnostic Dashboard] manifest.

## 3. Critical Failure Metrics (T2)
Every [T2 Capability](/docs/developer/pattern/doc-t2-capability.md) must define at least one "Critical Failure Metric" in its Operational Context section.
- **Example:** "Transaction failure rate exceeds 2% over a 5-minute window."
