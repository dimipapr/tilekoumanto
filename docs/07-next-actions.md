# Next Actions

## Current focus

Bootstrap a lightweight testing framework across the current MVP stack.

The goal is not broad coverage yet. The goal is to make it easy to add tests later once behavior is stable enough to protect.

## Immediate next actions

1. Define the minimal device telemetry decision behavior that should be stable now.

   Start with FreeRTOS-free C logic only:
   - first sample publishes
   - unchanged sample before timeout does not publish
   - mains power change publishes
   - pump relay change publishes
   - timeout elapsed publishes

   Lock this behavior down with C tests before adding new device features.