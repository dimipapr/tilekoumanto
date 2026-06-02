# Next Actions

## Current focus

Clarify and protect the basic device telemetry publishing model.

## Immediate next actions

1. Decide the next small boundary around publish-selected telemetry.

   Current known behavior:

   - telemetry selected for publishing gets `meta.seq`
   - skipped samples do not increment `seq`
   - `last_published` currently means last successful publish
   - retry, pending, in-flight, and publisher-task behavior are not settled yet

2. Add focused tests around the current sequence behavior before changing the publish architecture.

3. Do not document or implement queue/publisher-task behavior until the model is clearer.