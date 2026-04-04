---
id: cpp-incomplete-type-resolution-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](/docs/governance/standard/readme.md) > Standard: C++ Incomplete Type Resolution

# Standard: C++ Incomplete Type Resolution

This standard defines the strategy for resolving "incomplete type" compilation errors, particularly when integrating complex SDKs like OpenTelemetry.

## 1. Context
- **Symptom**: `invalid use of incomplete type` or `deletion of pointer to incomplete type` (triggering `-Wdelete-incomplete`).
- **Cause**: Using `std::unique_ptr` or `std::shared_ptr` on a type that is forward-declared but whose full definition is missing from the translation unit where it is destroyed.

## 2. PADU Evaluation

| Pattern | Rating | Nuance |
| :--- | :--- | :--- |
| **Explicit SDK Header Inclusion** | **P** | **Preferred.** Always include the specific SDK implementation header (e.g., `sdk/trace/processor.h`) in the `.cpp` where instances are managed. |
| **Opaque Pointers (PIMPL)** | **A** | **Alternative.** Useful for keeping headers clean, but requires a dedicated implementation file to own the full type visibility. |
| **Move Semantics Conversion** | **A** | **Alternative.** Moving `std::shared_ptr` into SDK-specific wrappers (like `nostd::shared_ptr`) requires both types to be fully defined. |
| **Implementation Forward-Decs** | **U** | **Unstable.** Relying on forward declarations in implementation files for types managed by smart pointers leads to fragile builds. |

## 3. Implementation Example (OpenTelemetry SDK)
```cpp
// Correct: Include both API and SDK headers in Telemetry.cpp
#include <opentelemetry/trace/provider.h>     // API
#include <opentelemetry/sdk/trace/processor.h> // SDK (Provides complete type)
```
