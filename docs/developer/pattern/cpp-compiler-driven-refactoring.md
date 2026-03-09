---
id: cpp-compiler-driven-refactoring
type: pattern
tags: [workflow, build, refactoring]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Compiler-Driven Refactoring

# Pattern: Compiler-Driven Refactoring

**Intent:** Let the C++ compiler act as the source of truth during wide-reaching structural changes to guarantee all downstream usages are correctly updated, bypassing the unreliability of global string replacement.

## Shape

### 1. Header-First Iteration Loop
Begin refactoring by altering the root abstraction boundary—such as an interface signature or core data structure in a header file. 

```bash
# 1. Change the signature in the header (e.g., RenderWindow -> RenderTarget)
# 2. Run the build to find all usages reliably.
make SpaceGameCore -j4
# 3. Fix the specific compilation errors reported.
# 4. Repeat until the build succeeds.
```

### 2. Avoid Blind Text Replacement
Do not rely on IDE global "Find and Replace" or regex scripts across the entire codebase to perform structural C++ changes. Blind renaming often misses instance calls (e.g., `window.draw()` changing to `target.draw()`) inside function bodies or incorrectly modifies shadowed generic variables.

## Key Constraints
- **Strict Typing as an Asset:** In strongly-typed languages like C++, compiler errors are exhaustive. Trust the build output over manual inspection.
- **Top-Down Propagation:** Always modify the headers/base definitions first and let the errors propagate downwards to the implementation files.

## Applied In
- Structural Rendering Modifications (sf::RenderTarget refactor)
