---
id: developer-standard-manifest
type: manifest
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > Standard

# Sub-pillar: Standard

PADU matrices rating patterns for operational fitness.

> [!NOTE]
> The [Operational Stability Standard](operational-stability-standard.md) in this directory serves as the developer-facing companion to the governance standards in [docs/governance/standard/](../../governance/standard/readme.md). It provides context-specific PADU ratings for high-frequency gameplay iteration patterns.

## Operational Stability
| Pattern | Purpose | P | A | D | U |
| :--- | :--- | :--- | :--- | :--- | :--- |
| [operational-stability-standard](operational-stability-standard.md) | Enforce header hygiene and explicit namespaces. | 1 | 1 | 5 | 5 |

## Project Construction & Build
| Pattern | Purpose | P | A | D | U |
| :--- | :--- | :--- | :--- | :--- | :--- |
| [cpp-standard-alignment](../pattern/cpp-standard-alignment.md) | Enforce C++20 and strict CMake flags. | 5 | 5 | 5 | 5 |
| [cpp-centralized-typedefs](../pattern/cpp-centralized-typedefs.md) | Centralize shared hashes/types in GameTypes.h. | 5 | 4 | 5 | 5 |
| [cpp-component-registration](../pattern/cpp-component-registration.md) | Ensure all .cpp files are in CMakeLists.txt. | 5 | 5 | 5 | 5 |

*P: Preferred, A: Acceptable, D: Discouraged, U: Unacceptable (Scale 1-5)*
