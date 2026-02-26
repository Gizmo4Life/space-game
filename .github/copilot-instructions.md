# Repository Architect Directives

## 1. Governance Routing
Every interaction must adhere to the [Governance Pillar](/docs/governance/readme.md). Follow the core protocols:
- **Discovery**: Repository-wide drift analysis.
- **Greenfield**: Pattern-first implementation.
- **PR Review**: Pillar-based verification.

## 2. Structural Purity
- **Atomicity**: Patterns = "What", Standards = "Should". extract inlined patterns.
- **One-Deep Rule**: Maintain singular directory depth. No nested subfolders.
- **Readability**: README paragraphs must not exceed 2 sentences.

## 3. Mandatory Verification
No change is complete without executing the modular sub-protocols:
- **Architecture**: Verify T2/T3 alignment.
- **Testing**: Define acceptance criteria before implementation.
- **Observability**: Plan and verify telemetry (Spans/Probes) emission.

## 4. RAG Optimization
Ensure every `.md` file has YAML frontmatter. Manifests must conclude with an `index_map` YAML block for automated traversal.