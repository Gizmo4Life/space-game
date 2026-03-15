---
id: build-env-network-isolation
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Network-Isolated Build Fallback

# Standard: Network-Isolated Build Fallback

**Context**: Development environments with restricted or unreliable internet access preventing the acquisition of external dependencies (e.g., Catch2, OpenTelemetry SDK).

## PADU (Preferred, Alternative, Discouraged, Unstable)

| Context | Preferred (P) | Alternative (A) | Discouraged (D) | Unstable (U) |
| :--- | :--- | :--- | :--- | :--- |
| **Dependency Acquisition** | Pre-installed local binaries | Bundled vendor submodules | FetchContent (Network Dependent) | Manual `wget/curl` during build |
| **Testing** | Standalone Verification Tools | Mocks of test frameworks | Skipping tests entirely | Commenting out test code |
| **Verification** | CIDR-compliant local runners | JSON/Text report output | Console-only logging | No verification |

## Guidelines

1. **Standalone Verification**: When Catch2 is unavailable, logic critical to the "Definition of Done" must be extracted into a standalone `int main()` utility located in `/scripts/` or a specialized verification target.
2. **Deterministic Output**: Verification tools must return `0` on success and non-zero on failure to ensure compatibility with script-based orchestration.
3. **Registry of Verified Logic**: Maintain a documentation signpost in the `walkthrough.md` or a `tests/readme.md` that maps standalone scripts to the components they verify.
