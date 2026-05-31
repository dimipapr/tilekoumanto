# Next Actions

## Current mode

Start adding nice shit

## 1. Resume MQTT ingress cleanup

Refactor MQTT ingress without changing behavior first.

Focus on making these steps explicit and testable:

- topic parsing
- device lookup
- payload validation
- device timestamp conversion
- raw message storage
- typed `PumpStateSample` creation

Do not switch latest-state ordering to `device_timestamp` until timestamp validation rules are defined and tested.