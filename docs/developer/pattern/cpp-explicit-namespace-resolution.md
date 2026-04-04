---
id: cpp-explicit-namespace-resolution
type: pattern
tags: [cpp, build, stability]
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > C++ Explicit Namespace Resolution

## Intent
Prevent name collisions and ambiguity when multiple libraries or local modules use similar naming conventions, specifically within the EnTT and SFML ecosystems.

## Motivation
The project uses `entt` for ECS. In some contexts, local namespaces or legacy code might define `entt` or similar symbols, leading to compiler errors like "No type named 'registry' in namespace 'entt'". Using explicit global resolution (`::entt`) removes this ambiguity.

## Rules
1. **Global Prefix for Core Libs**: Always use `::entt::` when referencing the EnTT registry or components in UI or system-level code.
2. **Avoid `using namespace`**: Never use `using namespace` in header files. Use it sparingly in `.cpp` files, and never for large libraries like `sf` or `entt` if possible.
3. **Fully Qualified Types**: In header files, always use fully qualified names for third-party types to avoid context-dependent resolution errors.

## Examples

### Bad (Ambiguous)
```cpp
void render(sf::RenderWindow &window, entt::registry &registry) { ... }
```

### Good (Explicit)
```cpp
void render(sf::RenderWindow &window, ::entt::registry &registry) { ... }
```
