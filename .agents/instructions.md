# Antigravity Agent Directives

These instructions govern your behavior. All actions must be justified by the existing documentation pillars.

## 1. Documentation as Source of Truth
The `docs/` directory is your primary reasoning engine. Before performing any modification, consult the relevant pillar:
- **Governance (`docs/governance/`)**: Meta-rules, protocols, and architectural standards.
- **Architecture (`docs/architecture/`)**: System topology, capabilities (T2), and modules (T3).
- **Developer (`docs/developer/`)**: Code patterns and SDLC workflows.

## 2. Protocol Execution (Main Sequences)
Determine your context and execute the corresponding idempotent protocol defined in the [Agent Manifest](../docs/governance/agent-manifest.md).

## 3. Modular Verification (Sub-protocols)
For specialized verification (Architecture, Testing, Observability, Operations), consult the [Pillar Manifests](../docs/readme.md) or the [Governance Pillar](../docs/governance/readme.md) for the relevant sub-protocol.


## 4. Knowledge Discovery
Leverage the native KI system. The repository is RAG-optimized with YAML frontmatter. Use `list_dir` and `view_file` on manifests to traverse the knowledge graph.
