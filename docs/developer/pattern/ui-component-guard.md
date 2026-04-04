---
id: ui-component-guard
type: pattern
pillar: developer
category: ux
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > UI Component Guard

# Pattern: UI Component Guard

A defensive access pattern for entity components in rendering code, using nullable lookups instead of direct access to gracefully handle entities missing expected components.

## Structure

1. Before accessing a component, obtain a nullable pointer (e.g., `try_get<T>`).
2. If the pointer is null, skip rendering or display a fallback placeholder.
3. No exceptions or assertions are triggered for missing components.
