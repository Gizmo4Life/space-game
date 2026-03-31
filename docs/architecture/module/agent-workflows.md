---
id: agent-workflows
type: module
pillar: architecture
---
[Home](/) > [Architecture](/docs/architecture/readme.md) > [Module](readme.md) > Agent Workflows

# Module: Agent Workflows

Automation definitions for repository-level protocols.

## 1. Physical Scope
- **Path:** `/.agent/workflows/`
- **Ownership:** Repository Architects

## 2. Capability Alignment
- [Agent Orchestration](/docs/architecture/capability/agent-orchestration.md)

## 3. Pattern Composition
- [Signpost Readme](/docs/developer/pattern/signpost-readme.md) (Standard: P)
- [agent-protocol-delegation](/docs/developer/pattern/agent-protocol-delegation.md) (P) — All workflow files delegate to governance protocol docs.
- [agent-iterative-compliance-loop](/docs/developer/pattern/agent-iterative-compliance-loop.md) (P) — Ralph Wiggum Loop gap-fix-verify cycle.
- [agent-dual-role-orchestration](/docs/developer/pattern/agent-dual-role-orchestration.md) (A) — Primary Agent → RWL executor handoff.

## 4. Telemetry
- **Probe:** `agent.workflow.load`
