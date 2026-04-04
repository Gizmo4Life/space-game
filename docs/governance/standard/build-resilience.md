---
id: build-resilience-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](/docs/governance/standard/readme.md) > Standard: Build Resilience Hub

# Standard: Build Resilience Hub

This hub indexes context-specific standards designed to maintain build health and resolve recurrent failures identified during the [Unified Change Protocol](../protocol/unified-change.md).

## 1. Objective
Build resilience is not a "silver bullet." It requires applying the correct code style or environmental strategy based on the specific failure context.

## 2. Contextual Standards Index

### Environment & Platform
- **[Build Environment (Mac/Network)](build-env-mac-restriction.md)**: Handling filesystem permission blockers and network-isolated dependency acquisition.

### Code Style & Testability
- **[C++ Test Visibility](cpp-test-visibility.md)**: Standards for promoting internal members to public for verification.
- **[C++ Incomplete Type Resolution](cpp-incomplete-type-resolution.md)**: Strategies for SDK integration and pointer safety.

### Integrity & Dependencies
- **[CMake Registration Consistency](cmake-registration-consistency.md)**: Rules for deterministic source registration and dependency acquisition.

## 3. Self-Correction Loop
Every build or test failure encountered must be analyzed via the **[Pattern Intake Protocol](../protocol/pattern-intake.md)** to determine which specific sub-standard needs expansion or if a new contextual standard is required.
