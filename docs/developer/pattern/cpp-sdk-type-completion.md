---
id: cpp-sdk-type-completion
type: pattern
tags: [cpp, memory, sdk]
---

# Pattern: SDK Type Completion

A structural pattern for managing types where a smart pointer requires a complete definition for deletion or conversion, but the public interface only provides a forward declaration.

## 1. Description
A management class uses a smart pointer (e.g., `std::shared_ptr` or `std::unique_ptr`) to own an implementation type. To satisfy the compiler's requirement for a complete type (e.g., for `operator delete` or type-erased conversion), the implementation header is explicitly included in the translation unit where the object is instantiated or managed, while the header remains minimal.

## 2. Structure
1.  **Interface Declaration**: A header file provides a forward-declaration of the implementation type.
2.  **Management Implementation**: A source file includes the full SDK/Implementation header.
3.  **Explicit Ownership Acquisition**: The object is instantiated (or cast) using a constructor that accepts a raw pointer or `std::move` of a smart pointer, ensuring the complete type's destructor/vtable is visible during the operation.

## 3. Generic Example
```cpp
// Implementation File
#include "Interface.h"
#include <sdk/implementation.h> // Complete Type Visibility

void Manager::init() {
    auto raw = new SDKImplementation();
    this->owned = std::shared_ptr<APIInterface>(raw);
}
```
