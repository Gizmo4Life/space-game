---
id: universal-runbook
type: universal_runbook
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Runbook](readme.md) > Universal System Runbook

# Universal System Runbook

This is the primary entry point for all on-call developers responding to an alert.

## 1. Severity Matrix
| Severity | Threshold | Response Time |
| :--- | :--- | :--- |
| **SEV-1** | > 10% Error Rate or Total Outage | 15 Mins |
| **SEV-2** | > 1% Error Rate or 2x Latency | 30 Mins |
| **SEV-3** | Degraded Performance | 4 Hours |

## 2. Impact Scope
- **SEV-1**: Universal service failure; all users blocked.
- **SEV-2**: Significant degradation; subset of users or core features affected.

## 3. System Orientation
- **Global Health Dashboard:** [Link to Main Dashboard]
- **Latency Overview:** [Link to Latency Dashboard]
- **Error Rates Overview:** [Link to Error Dashboard]

## 2. Triage Matrix
- **Strategy:** Adhere to the [OPS Triage Path](/docs/developer/pattern/ops-triage-path.md).
- **Action:** Identify the failing span from the dashboard and proceed to the corresponding Span Runbook.


## 3. Global Escalation
- **Strategy:** Adhere to the [OPS Escalation Path](/docs/developer/pattern/ops-escalation-path.md).
- **Action:** If triage fails:
  - **Primary On-Call:** [Name/Contact]
  - **Engineering Leadership:** [Name/Contact]

