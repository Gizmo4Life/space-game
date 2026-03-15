---
id: ops-scoped-resource-discovery
type: pattern
tags: [ops, research, search]
---

# Pattern: Scoped Resource Discovery

A search strategy that minimizes latency and permission flakes by prioritizing high-probability, high-permission scopes before expanding to global searches.

## 1. Description
A discovery agent initiates a search for a resource (file, symbol, or configuration) by targeting a specific, known-location subset. The search space is expanded only upon failure, with each step weighed against the cost of the search (time, CPU, or permission warnings).

## 2. Structure
1.  **Targeted Probe**: Search in the most likely directory or via a cached index.
2.  **Domain Expansion**: Search in standard system paths (e.g., `/usr/local`).
3.  **Global Fallback**: A system-wide search (`find /` or `mdfind`) executed only as a last resort with explicit boundary constraints.
4.  **Terminal Condition**: The search terminates when the object is found or a probability-of-success timeout is reached.

## 3. Generic Example
```bash
# Step 1: Targeted
ls /opt/homebrew/lib/target.so

# Step 2: Domain
find /usr/local -name "target.so"

# Step 3: Global (Restricted)
mdfind "name:target.so"
```
