---
id: external-manifest
type: manifest
pillar: external
---
[Home](/) > [Docs](/docs/readme.md) > External

# Pillar: External

System boundaries, API contracts, and third-party behavioral expectations.

```mermaid
graph LR
    Ext[External] --> UC[Use Cases]
    Ext --> Cont[Contract]
    Ext --> Int[Integration]
```

## Sub-directories
- [use-cases.md](use-cases.md): Primary operational contexts for repository consumption.
- [trading.md](trading.md): Guide for commodity trading and cargo volume management.
- [contract/](contract/): OpenAPI, GraphQL, and public interface definitions.
- [integration/](integration/): SLAs and behavior for 3rd party vendors.

---
## Machine Navigation Metadata
```yaml
type: directory_manifest
pillar: external
index_map:
  use_cases:
    path: use-cases.md
    scope: Primary repository consumption contexts.
  contract:
    path: contract/
    scope: Public API definitions.
  integration:
    path: integration/
    scope: Vendor SLAs and behavioral specs.
  player_manual:
    path: integration/player-manual.md
    scope: Core game mechanics and fleet logistics guide for players.
```

