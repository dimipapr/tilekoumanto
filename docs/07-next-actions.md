# Next Actions

## Current mode

Documentation reconciliation.

The implementation is ahead of the refreshed docs. First update the docs to match what already exists, then continue implementation.

## 1. Review operator tooling

Read:

- `operator/project.py`
- `operator/lib/certs.py`

Document:

- what it generates
- where files are written
- what is local-only
- what is ignored by git
- how device certificates relate to MQTT/mTLS

## 2. Review local deployment

Read:

- `backend/compose.yml`
- `backend/mosquitto/config/mosquitto.conf`
- `backend/caddy/etc/Caddyfile`
- relevant Django settings

Document:

- which services run locally
- how Django, PostgreSQL, Mosquitto, and Caddy connect
- whether MQTT is cleartext or TLS locally
- what is local-dev-only vs intended production shape

## 3. Update docs

Update:

- `03-current-state.md`
- `05-architecture.md`
- `04-decisions.md`
- `08-open-questions.md`

Keep current facts, decisions, and open questions separate.

## 4. Check API contract

Compare:

- `docs/contracts/openapi.yaml`
- `backend/django/devices/views.py`
- `backend/django/devices/urls.py`
- `backend/django/devices/models.py`

Document mismatches before changing code.

## 5. Resume implementation

After docs match the current system:

- refactor MQTT catcher
- tighten Pydantic MQTT contracts
- add ingress tests
- define stale-state behavior
- decide raw-message retention
- align latest-state API with OpenAPI

## Valid stop point

Safe to pause when:

- current state is accurate
- architecture matches implementation
- decisions are recorded
- open questions are listed
- this file shows the next small action
- repo has no uncommitted changes