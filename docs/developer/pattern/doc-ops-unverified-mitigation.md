---
id: doc-ops-unverified-mitigation
type: pattern
tags: [anti-pattern, operational]
category: anti-pattern
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Unverified Mitigation

# Anti-pattern: Unverified Mitigation Scripts

Providing restoration steps or scripts without an accompanying technical verification check.

## Why it's Forbidden
- **False Confidence:** The operator may assume the fix worked when it actually failed silently.
- **State Corruption:** Running blind fixes can compound system errors.

## Corrective Action
Ensure every [Restoration Step](/docs/developer/pattern/doc-ops-restoration-step.md) includes a mandatory "Technical Verification" block with expected outputs.
