# Next Actions

## Current focus

Bootstrap a lightweight testing framework across the current MVP stack.

The goal is not broad coverage yet. The goal is to make it easy to add tests later once behavior is stable enough to protect.

## Immediate next actions

1. Add or confirm backend Django test execution.

   Add one minimal sample test under the Django devices app.

   The sample test should prove the Django test runner works without locking down unstable behavior.

2. Add or confirm Python simulator test execution.

   Add one minimal sample test for simulator-side Python code.

   The sample test should avoid depending on live MQTT, certificates, Docker services, or random simulator behavior.

3. Add or confirm C device-core test execution.

   Add one minimal sample test for FreeRTOS-free C code.

   The sample test should establish the C test build/run path before adding real telemetry policy tests.

4. Decide where integration tests will live.

   Do not implement full-stack integration tests yet.

   Create only a placeholder or documented location for later tests involving MQTT, Docker Compose, mTLS, and backend ingestion.

5. After the framework exists, decide which current behaviors are stable enough to test.