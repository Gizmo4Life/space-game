---
id: agent-orchestration
type: capability
pillar: architecture
---
[Home](/) > [Architecture](/docs/architecture/readme.md) > [Capability](readme.md) > Agent Orchestration

# Capability: Agent Orchestration

## 1. Business Intent
Orchestrate the interaction between external AI agents and the repository's human-oriented documentation, enabling autonomous protocol execution without duplicating knowledge.

## 2. Orchestration Flow
1. **Directive Injection:** Agent reads `instructions.md` and `copilot-instructions.md` to load persona and behavioral rules.
2. **Context Retrieval:** Agent traverses the knowledge graph via RAG-optimized patterns and standards to ground its reasoning.
3. **Workflow Execution:** Slash commands map to workflow `.md` files in `/.agent/workflows/`, which delegate to governance protocols for multi-step operations.

## 3. Data Flow & Integrity
- **Trigger:** User slash command or inline agent invocation.
- **Output:** Protocol execution results; updated documentation artifacts.
- **Consistency:** Workflows are idempotent — safe to re-run if interrupted.

## 4. Operational Context
- **Primary Modules:** [agent-workflows](/docs/architecture/module/agent-workflows.md), [ai-config](/docs/architecture/module/ai-config.md) (T3)
- **Critical Failure Metric:** Workflow referencing a protocol that no longer exists.
