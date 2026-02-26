---
id: doc-ops-task
type: pattern
tags: [meta, operational, execution]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ops Task

## Structure
- **YAML Frontmatter:** Must include `type: task`.
- **1. Identification:** Links to the failing [T3 Module] and the triggering telemetry symptom.
- **2. Action (The Fix):** A copy-pasteable CLI command or script block.
- **3. Verification:** A telemetry query command and the expected success output.