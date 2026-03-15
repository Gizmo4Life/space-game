---
id: cpp-visibility-promotion
type: pattern
tags: [cpp, testing, architecture]
---

# Pattern: Visibility Promotion

Increasing the access level of a class member (e.g., from `private` to `public`) to allow external verification or manipulation.

## 1. Description
Members originally encapsulated for internal use are moved to the public interface to facilitate unit testing or observability when other injection/mocking strategies are unavailable or overly complex.

## 2. Structure
1.  **Encapsulated Member**: A variable or function located in a `private` or `protected` block.
2.  **Promotion**: Moving the member declaration to the `public` section of the class.
3.  **Observation**: External test runners or diagnostic tools access the member directly.

## 3. Implementation Example
```cpp
class System {
public:
  // Promoted for testability
  void internalRefresh(); 

private:
  int data_;
};
```
