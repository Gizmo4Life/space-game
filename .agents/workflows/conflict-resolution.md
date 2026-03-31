---
description: Resolve merge conflicts by preserving semantic intent and verifying standards.
---

# Workflow: Merge Conflict Resolution

This workflow automates the **[Merge Conflict Resolution Protocol](/docs/governance/protocol/merge-conflict-resolution.md)** for ensuring repository integrity.

## Instructions

1. **Detect Conflicts**:
   Scan the repository for merge fences.
   ```bash
   grep -r "<<<<<<<" .
   ```

2. **Analyze Both Sides**:
   Read the conflicting file and identify the `HEAD` and `Incoming` changes.

3. **Determine Resolution Goal**:
   - if it's a **Simple Additive** conflict (both added new sections/functions), **Keep Both** and re-order them logically.
   - if it's a **Functional Displacement** conflict (one changed a function, another changed its usage), **Refactor or Reconcile** to ensure compatibility.
   - if it's a **Duplicate** conflict (both added the same thing but differently), **Select the Most Standards-Compliant** version and integrate missing details.

4. **Verify Standards**:
   Ensure the resolved content follows the **[Definition of Done](/docs/developer/pattern/definition-of-done.md)**.

5. **Verify Correctness**:
   - For documentation: Validate links and formatting.
   - For code: Ensure it builds and passes tests.

> [!IMPORTANT]
> Never blindly pick `ours` or `theirs` without performing semantic analysis.
