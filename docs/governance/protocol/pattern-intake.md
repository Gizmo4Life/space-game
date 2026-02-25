---
id: pattern-intake
type: protocol
pillar: governance
---
[Home](/) > [Governance] > [Protocol] > Pattern Intake

## 1. Objective
Extract a novel code or document structure, define it as a contextless geometry [Pattern], and assign its contextual fitness in a [Standard].

## 2. Extraction & Decontextualization
- **Action:** Analyze the target code or document. Strip away all domain-specific variables, business logic, and contextual "opinions."
- **Verify:** The remaining structure must be a pure "Platonic shape" (e.g., "A class that implements an interface and returns a Result object").

## 3. Pattern Materialization
- **Action:** Create a new file in `docs/developer/pattern/` using expressive, singular naming (e.g., `data-repository-abstraction.md`).
- **Action:** Include the standard YAML frontmatter (`id`, `type: pattern`, `tags`).
- **Verify:** The file contains absolutely no "best practice" or "should/must" language regarding its usage.

## 4. Shadow Indexing
- **Action:** Open `docs/developer/pattern/readme.md`.
- **Action:** Add the new pattern under the most relevant human-facing Concern heading.
- **Action:** Update the YAML `index_map` at the bottom of the README to ensure RAG pipelines can traverse it.

## 5. Contextual Judgment (PADU)
- **Action:** Identify the [Standard] (in `docs/developer/standard/` or `docs/governance/standard/`) that governs the context where this pattern was found.
- **Action:** Insert the pattern into the Standard's Markdown table.
- **Action:** Assign a rating (Preferred, Acceptable, Discouraged, Unacceptable) and write the specific **Nuance** justifying the rating for that context.