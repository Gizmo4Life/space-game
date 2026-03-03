---
id: cpp-standard-alignment
type: pattern
tags: [cpp, build, standard]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > C++ Standard Alignment

## Problem
Mismatches between the compiler standard (e.g., C++20) and linter settings/extensions can cause "structured binding" errors and "C++17 extension" warnings, even when the project intended to support them.

## Structure
- **Requirement:** The C++ standard must be explicitly set and enforced in `CMakeLists.txt` using `CMAKE_CXX_STANDARD 20` and `CMAKE_CXX_STANDARD_REQUIRED ON`.
- **Requirement:** `CMAKE_CXX_EXTENSIONS` should be set to `OFF` to ensure portability and avoid confusion with compiler-specific extensions.
- **Guideline:** Avoid structured bindings in tight loops or where portability is paramount, or ensure the build environment (CI and Local) is strictly aligned on the compiler version.

## Verify
- `grep "CMAKE_CXX_STANDARD 20" CMakeLists.txt`
- Check for "extension" warnings in build logs.
