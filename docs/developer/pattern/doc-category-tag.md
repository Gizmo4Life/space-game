---
id: doc-category-tag
type: pattern
pillar: developer
category: geometry
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Category Tag

# Pattern: Category Tag

## 1. Problem
In a flat pattern directory (142+ files), machine consumers and agents cannot determine a pattern's domain without parsing its title or content. Human readers scanning logs or search results lack immediate context for what category a pattern belongs to.

## 2. Solution
Every pattern file's YAML frontmatter includes a `category` field drawn from a fixed vocabulary. The canonical vocabulary is maintained in the Category Index at the bottom of [docs/developer/pattern/readme.md](readme.md).

### Frontmatter Example
```yaml
---
id: economy-dynamic-pricing
type: pattern
pillar: developer
category: engine
---
```

### Canonical Categories
| Category | Scope |
|:---|:---|
| `elicitation` | Transforming vague intent into testable constraints |
| `geometry` | Skeletal structure — how files map to the Knowledge Graph |
| `logic` | Code correctness, verification, and structural integrity |
| `cicd` | Automated deployment, container, and build pipeline |
| `ops` | System uptime, triage, incident response, and restoration |
| `engine` | C++ game engine shapes — physics, ECS, economy, faction AI |
| `ux` | Machine-readability, human scannability, and UI rendering |
| `anti-pattern` | Forbidden or legacy shapes that trigger audit failures |
| `agent` | AI-assisted workflow automation and compliance loops |

## 3. Constraints
- The `category` value **must** be one of the canonical values listed above.
- Each pattern belongs to **exactly one** category (no multi-assignment).
- New categories require updating both the canonical table above and the Category Index in the pattern readme.

## 4. Traceability
- **Standard:** [Document Organization](/docs/governance/standard/document-organization.md) (P)
- **Index:** [Pattern Readme — Category Index](readme.md)
