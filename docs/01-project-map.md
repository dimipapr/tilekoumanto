# Project Map

## Project

Tilekoumanto - remote visibility and control for agricultural pump installations.

## Product focus

The product targets Greek farmers with irrigated fields that use on-site three-phase water pumps controlled locally through a manual start/stop circuit.

The first product direction is remote visibility, not remote control.

## Current problem focus

Farmers lose visibility into the irrigation system after leaving the field. They may not know whether mains power is available, whether the pump is running, or whether irrigation is continuing as expected.

The initial focus is on reducing uncertainty and unnecessary trips made only to check the pump or power state.

## MVP

The MVP is a monitoring-only version for one field and one pump.

It includes:

- remote visibility of mains power state
- remote visibility of pump relay state

It does not include remote start/stop control, pressure monitoring, automation, scheduling, or multi-field management.

## Current documentation

- `01-project-map.md` is this file. It provides the project orientation and restart point.
- `02-product-definition.md` defines the problems, proposed solution direction, and MVP scope.
- `03-current-state.md` describes what currently exists in the implementation.
- `04-decisions.md` records important product, architecture, and implementation decisions.
- `05-architecture.md` describes the system architecture, components, data flow, and interfaces.
- `06-implementation-log.md` records chronological work progress and changes.
- `07-next-actions.md` defines the next small implementation and documentation tasks.
- `08-open-questions.md` tracks unresolved questions that need research, testing, or decisions.
- `09-device-subsystem.md` field device design architecture and other thingies
- `10-scratchbook.md` is for temporary notes, rough ideas, and unsorted thoughts.

## Stop point

Before stepping away from the project, make sure the project can be resumed without reconstruction.

A valid stop point requires:

- `03-current-state.md` reflects what currently works, what is broken, and what is unknown.
- `06-implementation-log.md` has a latest entry describing what changed in the last work session.
- `07-next-actions.md` contains the next small action to take.
- Any unresolved uncertainty is captured in `08-open-questions.md`.
- `10-scratchbook.md` has been reviewed, with useful notes moved into the correct document and obsolete notes removed.
- The repository has no uncommitted changes.

The project is safe to pause when future work can continue by reading `01-project-map.md`, `03-current-state.md`, and `07-next-actions.md` without needing to reconstruct context from scratch.

## Current project phase

Cant really update it every time...

## Resume here

When returning to the project:

1. Read this file.
2. Read `02-product-definition.md`.
3. Read `03-current-state.md`.
4. Read `07-next-actions.md`.
5. Continue from the first item in `07-next-actions.md`.

## Current next step

Fill `03-current-state.md` with what already exists in the implementation.