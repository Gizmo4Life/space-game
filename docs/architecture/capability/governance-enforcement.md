---
id: governance-enforcement
type: capability
pillar: architecture
---
[Home](/) > [Architecture](/docs/architecture/readme.md) > [Capability](readme.md) > Governance Enforcement

# Capability: Governance Enforcement

## 1. Business Intent
Maintain architectural integrity by formally validating that all repository patterns, standards, and protocols are applied consistently and that documentation never drifts from the codebase.

## 2. Orchestration Flow
1. **Structural Audit:** Verify breadcrumbs, YAML frontmatter, and internal links via the Documentation Validation protocol.
2. **Drift Detection:** Scan `src/` and `docs/` for undocumented code, missing T3 modules, and ghost logic (patterns rated U/D in standards).
3. **Standard Enforcement:** Verify PADU ratings are assigned and that all T3 modules link to at least one standard.
4. **Reconciliation:** Create or update T3 module files to align with discovered code shapes.

## 3. Data Flow & Integrity
- **Trigger:** Manual run of the Discovery or Documentation Validation protocol, or PR gate.
- **Output:** Updated T2/T3 documentation; flagged violations.
- **Consistency:** Protocols are idempotent — safe to re-run after partial completion.

## 4. Operational Context
- **Primary Modules:** [governance-protocols](/docs/architecture/module/governance-protocols.md), [governance-standards](/docs/architecture/module/governance-standards.md), [developer-patterns](/docs/architecture/module/developer-patterns.md) (T3)
- **Critical Failure Metric:** Any T3 module exceeding 50 lines, or a ghost logic pattern reaching production.
