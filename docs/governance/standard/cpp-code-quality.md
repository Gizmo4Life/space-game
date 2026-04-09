---
id: cpp-code-quality
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > C++ Code Quality

# Standard: C++ Code Quality (PADU)

This standard governs foundational C++ structural and syntactic choices, ensuring clean encapsulation, tight namespace boundaries, and safe memory management across the codebase.

## 1. Type Safety & Identity
| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [cpp-centralized-typedefs](/docs/developer/pattern/cpp-centralized-typedefs.md) | **P** | Unified type definitions (`types.h`). | N/A |
| [cpp-type-safe-handles](/docs/developer/pattern/cpp-type-safe-handles.md) | **P** | Strict types instead of primitive ID passing. | N/A |
| [cpp-standard-alignment](/docs/developer/pattern/cpp-standard-alignment.md) | **P** | Targeting C++20 standard features exclusively. | N/A |
| **Raw Pointers for Ownership** | **U** | Use `std::unique_ptr` and EnTT handles. |

## 2. Modularity & Namespaces
| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [cpp-explicit-namespace-resolution](/docs/developer/pattern/cpp-explicit-namespace-resolution.md) | **P** | Prefer `game::System` over `using namespace`. | N/A |
| [cpp-interface-segregation](/docs/developer/pattern/cpp-interface-segregation.md) | **P** | Small, focused abstract classes. | N/A |
| [cpp-external-api-facade](/docs/developer/pattern/cpp-external-api-facade.md) | **P** | Wrap external deps to avoid type leakage. | N/A |
| [cpp-sdk-type-completion](/docs/developer/pattern/cpp-sdk-type-completion.md) | **A** | Providing full type traits if required. | Acceptable when bridging third-party SDKs (like OpenTelemetry) that demand full type resolution instead of incomplete opaque pointers. |
| **Global State** | **D** | External coupling without injection. |

## 3. Physical Layout (Headers & Aggregation)
| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [cpp-header-hygiene](/docs/developer/pattern/cpp-header-hygiene.md) | **P** | No extraneous includes or deep include trees. | N/A |
| [cpp-header-resilience](/docs/developer/pattern/cpp-header-resilience.md) | **P** | Use forward declarations over includes where possible. | N/A |
| [cpp-component-aggregation](/docs/developer/pattern/cpp-component-aggregation.md) | **P** | Master headers for logical component groups. | N/A |
| [cpp-component-registration](/docs/developer/pattern/cpp-component-registration.md) | **P** | Unified EnTT meta-registration loops. | N/A |
| [cpp-structural-integrity](/docs/developer/pattern/cpp-structural-integrity.md) | **P** | Maintain source-to-header coupling. | N/A |
| **Circular includes** | **U** | Fatal build errors. |

## 4. Architecture Implementation
| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [cpp-compiler-driven-refactoring](/docs/developer/pattern/cpp-compiler-driven-refactoring.md) | **P** | Leaning on strict typing to guide refactors. | N/A |
| [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) | **A** | Acceptable for non-entity global state. | Acceptable exclusively for structural hardware abstractions (e.g. Telemetry/Logger, FactionManager) that operate entirely outside the ECS graph. |
| **Deep Inheritance** | **U** | Prefer composition via ECS. |
