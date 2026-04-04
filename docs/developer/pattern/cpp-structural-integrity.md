---
id: cpp-structural-integrity
type: pattern
tags: [foundation, build, structural-hygiene]
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Structural Integrity

# Pattern: Structural Integrity

**Intent:** Prevent structural code corruption (e.g., nested function definitions, mismatched braces, or scope leakage) and API mismatches by enforcing a rigorous verification protocol.

## Shape

### 1. Scope Hygiene Protocol
When performing large-scale code replacements or refactors, agents must explicitly verify the start and end of the modified scope.

```cpp
// BAD: Accidental nesting caused by malformed replacement range
void outer() {
   void inner() { // Error: nested function
   }
}

// GOOD: Verified scope boundaries
namespace space {
void MyClass::myMethod() {
  // implementation
}
} // namespace space
```

### 2. Pre-flight Syntax Verification (FSyntax-Only)
Before considering an implementation task "complete," agents MUST run a syntax-only check on the modified file(s). This is faster than a full build and catches structural errors immediately.

```bash
# Verify structural integrity without full linking
c++ -std=c++20 -Isrc -fsyntax-only path/to/modified_file.cpp
```

### 3. API & Singleton Discovery
Before invoking a singleton or an unfamiliar module method, the agent MUST read the corresponding header (`.h`) to verify:
- **Access Patterns**: Is it a singleton? (`::instance()`)
- **Signatures**: Check return types, `const` qualifiers, and parameter lists.
- **Accessibility**: Ensure constructors or methods are not `private` or `protected`.

## Key Constraints
- **Zero Nesting**: Never define functions within functions in C++.
- **Overwrite for Corruption**: If a file's structure is compromised (e.g., massive nesting errors), prefer a full `write_to_file` overwrite over multiple `replace_file_content` calls.
- **Telemetry Consistency**: Always verify the case and return type of spans/tracers, as these often vary between libraries.

## Applied In
- `Operational Stability Standard`
- `ShipOutfitter.cpp`
