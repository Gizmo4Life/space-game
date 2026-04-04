---
id: docker-profile-gated-service
type: pattern
pillar: developer
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Docker Profile-Gated Service

# Pattern: Docker Profile-Gated Service

A Compose service that is excluded from the default `docker compose up` and only activated when its profile is explicitly requested, preventing unnecessary builds of resource-heavy targets.

## Structure

1. A **service definition** includes a `profiles: ["<name>"]` field.
2. The service is invisible to `docker compose up` and `docker compose ps` unless the profile is activated.
3. **Activation**: `docker compose --profile <name> build` or `docker compose --profile <name> up`.
4. Other services without profiles start normally and are unaffected.
