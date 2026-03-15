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
