---
id: header-management
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Header Management (IWYU)

# Standard: Header Management (Include-What-You-Use)

**Context**: C++ projects often suffer from "include bloat," where headers are included but not used, or used transitively through other headers. This slows down compilation and creates hidden dependencies.

## PADU (Preferred, Alternative, Discouraged, Unstable)

| Context | Preferred (P) | Alternative (A) | Discouraged (D) | Unstable (U) |
| :--- | :--- | :--- | :--- | :--- |
| **Direct Usage** | Include exactly what is used in the file. | Forward declarations for pointers/references. | Transitive inclusion (relying on other headers). | Including `catch_all.hpp` in non-test files. |
| **Cleanliness** | Zero unused headers (per linter). | Minimal necessary headers. | Unused headers with a comment explanation. | "Just in case" includes. |
| **Automation** | Automated IWYU tools (`clang-include-cleaner`). | Manual periodic audits. | No header check. | Disabling unused-header warnings globally. |

## Guidelines

1. **Direct Inclusion**: Every file should include the headers defining the symbols it uses directly. Do not rely on a header including another header you need.
2. **Unused Headers**: Headers that are not directly used MUST be removed. 
   - *Exception*: Library headers required for side-effects (e.g., global registration) must be explicitly commented with `// IWYU pragma: keep`.
3. **Forward Declarations**: Prefer forward declarations over full includes in header files whenever possible (e.g., when only pointers or references are used).

## Success State
A file is compliant if it contains zero unused headers and all used symbols are accounted for by direct includes or forward declarations.

## Post-Refactor Orphan Hygiene

When logic is centralized (e.g., removing an inline `registry.view<PlayerComponent>()` in favor of `findFlagship`), the header that previously defined the locally-used type becomes an **orphan include** — still present but no longer directly used.

| Scenario | Action |
| :--- | :--- |
| A `registry.view<T>()` loop is replaced by a centralized utility | Remove the header for `T` from the calling file if it is no longer used directly. |
| A multi-step component aggregation is replaced by `blueprintFromEntity` | Remove all `InstalledModules.h` etc. from the calling file unless another symbol from that header is still used. |
| A duplicate `#include` is introduced during a refactor merge | Remove the duplicate — C++ include guards prevent double-inclusion but duplicate lines are a maintenance hazard. |

**Protocol**: Every PR that centralizes logic MUST include a header hygiene pass on all modified files as part of the [Definition of Done](/docs/developer/pattern/definition-of-done.md).
