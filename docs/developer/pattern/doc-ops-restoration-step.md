---
id: doc-ops-restoration-step
type: pattern
tags: [meta, operational, restoration, task]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Restoration Step

# Pattern: Restoration Step

A standalone, reusable corrective action intended to restore a specific component or span to a healthy state.

## Structure
- **YAML Frontmatter:** Must include `type: restoration_step`.
- **1. Objective:** Short description of what this step accomplishes (e.g., "Clear Redis Cache").
- **2. Prerequisites:** Any conditions that must be met before execution (e.g., "VPN access required").
- **3. Execution (The Fix):** The exact commands, script blocks, or UI actions. Use copy-pasteable blocks for CLI tasks.
- **4. Technical Verification:** CLI commands or local checks to confirm the step's local success.
- **5. Rollback:** (Optional) How to undo the action if necessary.
