---
id: cicd-secret-vaulting
type: pattern
tags: [foundation, cicd, security]
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Secret Vaulting

# Pattern: Secret Vaulting

All sensitive credentials (API keys, DB passwords, SSH keys) must be stored in a dedicated, external vault and injected at runtime.

## Why it's Required
- **Prevention of Leakage:** Keeps secrets out of the source code and build logs.
- **Rotation:** Allows for credential rotation without modifying code or redeploying artifacts.

## Constraints
- **Forbidden:** No clear-text secrets in `env` files or CI configs.
- **Required:** Use JIT (Just-In-Time) token injection where possible.
