# Next Actions

## Current mode

Documentation reconciliation is mostly complete for the current backend/device ingestion path.

The next step is to commit the documentation updates, then resume implementation with the smallest missing MVP API endpoint.

## 1. Resume MQTT ingress cleanup

After the latest-state API contract is aligned:

* refactor MQTT catcher into smaller functions
* tighten or confirm Pydantic MQTT contracts
* add ingress tests
* decide unknown-device/raw-message retention behavior
