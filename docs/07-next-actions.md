# Next Actions

## Current mode

Documentation reconciliation is mostly complete for the current backend/device ingestion path.

The next step is to commit the documentation updates, then resume implementation with the smallest missing MVP API endpoint.

## 1. Commit documentation reconciliation updates

Commit the documentation updates covering:

- operator certificate tooling
- local Docker deployment
- backend device provisioning path
- MQTT ingestion behavior
- Django admin inspection
- current latest-state API gap
- remaining open questions

## 2. Implement latest-state API

Implement:

```text
GET /api/devices/{device_uuid}/state
````

Use the latest `PumpStateSample` for the requested `Device.uuid`.

Before or during implementation, decide the minimal MVP behavior for:

* unknown device UUID
* known device with no pump state samples
* `state_is_stale`

## 3. Add focused API tests

Add tests for:

* latest-state API response for a device with a pump state sample
* unknown device UUID behavior
* known device with no samples behavior
* stale-state behavior once defined

## 4. Resume MQTT ingress cleanup

After the latest-state API contract is aligned:

* refactor MQTT catcher into smaller functions
* tighten or confirm Pydantic MQTT contracts
* add ingress tests
* decide unknown-device/raw-message retention behavior

## Valid stop point

Safe to pause when:

* current state is accurate
* implementation log records the latest review
* open questions contain only unresolved items
* this file shows the next small implementation step
* repo has no uncommitted changes
