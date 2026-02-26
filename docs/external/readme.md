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
    Ext[External] --> Cont[Contract]
    Ext --> Int[Integration]
```

## Sub-directories
- [contract/](contract/): OpenAPI, GraphQL, and public interface definitions.
- [integration/](integration/): SLAs and behavior for 3rd party vendors.

---
## Machine Navigation Metadata
```yaml
type: directory_manifest
pillar: external
index_map:
  contract:
    path: contract/
    scope: Public API definitions.
  integration:
    path: integration/
    scope: Vendor SLAs and behavioral specs.
```
