---
id: cmake-registration-consistency-standard
type: standard
pillar: governance
---

# Standard: CMake Registration Consistency

This standard ensures that the build configuration accurately and deterministically reflects the source state and dependency requirements of the project.

## 1. Context
- **Symptom**: Linker errors, missing symbols, or compilation flakes due to network-dependent dependency fetching.
- **Challenge**: Balancing ease of dependency management with build reliability.

## 2. PADU Evaluation

| **Explicit Source Registration** | **P** | **Preferred.** Manually adding every `.cpp` to `CORE_SOURCES` prevents silent omission of logic from builds. |
| [cicd-hybrid-dependency-acquisition](/docs/developer/pattern/cicd-hybrid-dependency-acquisition.md) | **P** | **Preferred.** Ensures local dependencies are used when available, falling back to remote if enabled. |
| **Test Target Persistence** | **P** | **Preferred.** Ensure `SpaceGameTests` is always present and links all test files, even during refactors. |
| [cicd-fetchcontent-dependency](/docs/developer/pattern/cicd-fetchcontent-dependency.md) | **U** | **Unstable.** Pure network-dependent fetching is fragile in restricted or offline environments. |
| **Globbing (File discovery)** | **D** | **Discouraged.** Leads to non-deterministic builds and unintended inclusion of temporary or local-only files. |
| **Target Deletion** | **U** | **Unstable.** Temporarily removing `SpaceGameTests` during maintenance breaks CI visibility. Use conditional `BUILD_TESTING` instead. |

## 3. Maintenance Rule
Every time a new source file is created in `src/`, it must be registered explicitly in `CMakeLists.txt` during Step 0 (Pre-flight) of the Unified Change Protocol.
