---
id: getting-started
type: workflow
tags: [onboarding, setup]
---
[Home](/) > [Developer] > [Workflow] > Getting Started

## 1. Objective
To initialize a local development environment that passes the [Discovery Protocol](/docs/governance/protocol/discovery.md).

## 2. Prerequisites
- Docker Desktop > 4.0
- Node.js > 20 LTS

## 3. Procedure
1. **Clone & Signpost:**
   - Run `git clone ...`
   - Verify `readme.md` exists in root.
2. **Install Dependencies:**
   - Run `npm install`
3. **Hydrate Architecture:**
   - Run `npm run doc:generate` to build the T3 Module map.
4. **Verify Compliance:**
   - Run `npm run audit` to check for Governance violations.

## 4. Definition of Done
- `npm test` passes.
- No "Unacceptable" patterns detected in local scan.