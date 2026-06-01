# Open Questions


## Fault persistence

Faults are currently validated and retained in `DeviceMessageRaw.payload`.

Open question: should faults also be projected into a dedicated backend model?

## C telemetry fault model

The backend contract supports a fault list, but the current C `tk_telemetry_t` is still minimal.

Open question: how should fault details enter the C telemetry model?

## Simulator behavior model

The Python simulator currently uses random input generation.

Open question: how should deterministic scenarios be represented?

## STM32 identity storage

Open question: how should the embedded target store and load provisioned identity/cert material?

## STM32 logging

Open question: what should the STM32 logging sink be during bring-up and later production firmware?

## Backend fault API

Open question: if faults get a dedicated model, how should latest-state and historical APIs expose them?
