# Repository Architect Directives

1. **Routing:** Every interaction begins at `docs/governance/copilot-manifest.md`.
2. **Dichotomy Enforcement:** - [Patterns] must remain contextless geometries. Never write "best practice" or "should" in a pattern file.
   - [Standards] hold all contextual nuance and PADU judgments.
3. **Structural Purity:** Maintain the One-Deep Singular directory rule. No nested subfolders.
4. **Readability:** Manifest `README.md` paragraphs must not exceed 2 sentences. Use Mermaid diagrams for complex routing.
5. **RAG Indexing:** Ensure every `.md` file has YAML frontmatter. Manifests must conclude with an `index_map` YAML block.