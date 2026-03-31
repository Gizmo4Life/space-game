---
description: Execute the Documentation Refinement protocol for iterative pattern extraction, standards refinement, and architecture updates.
---

# Workflow: Documentation Refinement

This workflow automates the [Documentation Refinement Protocol](/docs/governance/protocol/documentation-refinement.md). To ensure strict adherence to the latest standards, you must first consult the source of truth.

## Instructions

1. **Consult Source Protocol:** Read [documentation-refinement.md](/docs/governance/protocol/documentation-refinement.md) and execute the steps exactly as defined in the **Extraction Pass**, **Standards Refinement**, **Architecture Update**, and **Integrity Check** sections.
2. **Execute Protocol:** Follow the protocol steps directly from the source file.
3. **Build & Test:** After each iteration, run the [RWL Build & Test Script](/scripts/governance/rwl-build-test.sh) to validate no documentation references are broken:
   ```bash
   // turbo
   ./scripts/governance/rwl-build-test.sh --build-only
   ```
4. **Iterate:** Repeat until the Integrity Check (Step 6) finds zero gaps.
