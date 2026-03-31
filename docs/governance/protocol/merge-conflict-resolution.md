---
id: merge-conflict-resolution
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Merge Conflict Resolution

## 1. Objective
Ensuring that merge conflicts are resolved by preserving the semantic intent of both conflicting versions, rather than destructive choosing or blind overwriting.

## 2. Identification
- **Action:** Execute `grep -r "<<<<<<<" .` to identify all files containing merge markers.
- **Action:** Identify the origin of the conflict (e.g., a feature branch merge vs. a doc update).

## 3. Semantic Analysis
- **Analyze HEAD:** What was the intent of the current version?
- **Analyze Incoming:** What is the intent of the incoming version?
- **Check for Complementarity:** Often, both versions specify necessary information that should be combined (e.g., one adds a feature, another adds telemetry for the same feature).

## 4. Resolution Steps
1. **Manual Integration:** Do not use auto-resolve tools if the conflict is in logic or architecture.
2. **Preserve Standards:** Ensure that the resolved code/doc still adheres to the [Definition of Done](/docs/developer/pattern/definition-of-done.md).
3. **Numbering & Hierarchy:** In documentation, ensure section numbers (e.g., ## 1, ## 2) remain sequential and hierarchical.
4. **Link Integrity:** Verify that any cross-links introduced in either branch are still valid in the merged state.

## 5. Verification
- **Doc Verification:** Run `grep` again to ensure all fences are removed. Verify formatting in a previewer if possible.
- **Code Verification:** If the conflict was in code, the project **MUST** build successfully.
- **Logic Verification:** Run relevant unit tests or viability tests to ensure the merge didn't break functionality.

## 6. Escalation
- If the semantic intent of both sides is fundamentally incompatible, escalate to the Architect or the USER for a design decision.
