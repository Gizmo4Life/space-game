# Antigravity Agent Directives

These instructions govern your behavior. All actions must be justified by the existing documentation pillars.

## 1. Documentation as Source of Truth
The `docs/` directory is your primary reasoning engine. Before performing any modification, consult the relevant pillar:
- **Governance (`docs/governance/`)**: Meta-rules, protocols, and architectural standards.
- **Architecture (`docs/architecture/`)**: System topology, capabilities (T2), and modules (T3).
- **Developer (`docs/developer/`)**: Code patterns and SDLC workflows.

## 2. Protocol Execution (Main Sequences)
You MUST follow the standardized protocols for all repository modifications:
- **Greenfield**: Use [greenfield.md](/docs/governance/protocol/greenfield.md) for new features.
- **Discovery**: Use [discovery.md](/docs/governance/protocol/discovery.md) for drift analysis.
- **PR Review**: Use [pull-request-review.md](/docs/governance/protocol/pull-request-review.md) for verification.
- **Triage**: Use [operational-triage.md](/docs/governance/protocol/operational-triage.md) for system health issues.
- **Intake**: Use [pattern-intake.md](/docs/governance/protocol/pattern-intake.md) for new patterns.

## 3. Modular Verification (Sub-protocols)
Apply these building blocks to every task to ensure compliance:
- **Architecture**: Execute [architecture-review.md](/docs/governance/protocol/architecture-review.md).
- **Testing**: Execute [test-planning.md](/docs/governance/protocol/test-planning.md).
- **Observability**: Execute [observability-compliance.md](/docs/governance/protocol/observability-compliance.md).
- **Operations**: Execute [runbook-completeness.md](/docs/governance/protocol/runbook-completeness.md).

## 4. Knowledge Discovery
Leverage the native KI system. The repository is RAG-optimized with YAML frontmatter. Use `list_dir` and `view_file` on manifests to traverse the knowledge graph.
