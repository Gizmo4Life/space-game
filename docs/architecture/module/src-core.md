---
id: src-core
type: module
pillar: architecture
dependencies: []
---
[Home](/) > [Architecture](/docs/architecture/readme.md) > [Module](readme.md) > Src Core

# Module: Src Core

Physical implementation of the core RAG logic and document processing engine.

## 1. Physical Scope
- **Path:** `/src`
- **Ownership:** Core Engineering Team

## 2. Capability Alignment
- [Doc Ingestion](/docs/architecture/capability/doc-ingestion.md)

## 3. Pattern Composition
- [Signpost Readme](/docs/developer/pattern/signpost-readme.md) (Standard: P)

## 4. Telemetry & Observability
- **Span:** `src.process.init`
- **Probe:** `src.health`
