# Next Actions

## Current mode

Documentation reconciliation is mostly complete for the current backend/device ingestion path.

The next step is to commit the documentation updates, then resume implementation with the smallest missing MVP API endpoint.

## 1. Align OpenAPI contract with implemented latest-state response

Update `docs/contracts/openapi.yaml` to match the implemented endpoint response:

- `device_reported_at`
- `backend_received_at`
- no backend-computed `state_is_stale`
- 404 behavior for unknown devices and known devices without samples

## 2. Resume MQTT ingress cleanup

After the latest-state API contract is aligned:

* refactor MQTT catcher into smaller functions
* tighten or confirm Pydantic MQTT contracts
* add ingress tests
* decide unknown-device/raw-message retention behavior
