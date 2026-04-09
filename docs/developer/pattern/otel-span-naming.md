---
id: otel-span-naming
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > OpenTelemetry Span Naming

# Pattern: OpenTelemetry Span Naming

**Context:** To ensure global searchability and consistent dashboarding within SigNoz, all telemetry spans must follow a predictable hierarchical dot-notation.

**Pattern:** `<pillar>.<module>.<action>`

**Examples:**
- `engine.physics.step`
- `game.combat.weapon.fire`
- `ai.navigation.path.calculate`
