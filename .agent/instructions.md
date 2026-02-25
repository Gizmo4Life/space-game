# Antigravity Agent Directives

These instructions govern your behavior in this repository. All actions must be justified by the existing documentation pillars.

## 1. Documentation as Source of Truth
The `docs/` directory is your primary reasoning engine. Before performing any structural modification or code implementation, you MUST consult the relevant pillar:
- **Governance (`docs/governance/`)**: For meta-rules, protocols, and architectural standards.
- **Architecture (`docs/architecture/`)**: For system topology, capabilities (T2), and modules (T3).
- **Developer (`docs/developer/`)**: For code patterns and SDLC workflows.

## 2. Protocol Adherence
When modifying the repository, you MUST follow the [Protocol Execution Standard](/docs/governance/standard/protocol-execution.md). 
- Always consult the specific [Protocol](/docs/governance/protocol/readme.md) file before starting a task.
- Ensure your actions are idempotent and keep documentation in sync with physical code.

## 3. Knowledge Discovery
Leverage your native Knowledge Item (KI) system. The repository is RAG-optimized with YAML frontmatter to facilitate this. Use `list_dir` and `view_file` on manifests (`readme.md`) to traverse the knowledge graph.
