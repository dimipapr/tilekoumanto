# Current State

## Purpose

This document records what currently exists, what has been verified, what is unverified, and what should happen next.

## Product scope currently in force

The current MVP is monitoring-only for one field and one pump.

Included:

- remote visibility of mains power state
- remote visibility of pump relay state
- simple API/interface showing current state

Excluded for now:

- remote start/stop control
- pressure monitoring
- notifications
- automation/scheduling
- multi-field management
- separate web or mobile dashboard

## Repository state observed

The repository currently contains:

- Docker Compose backend stack
- Django backend project
- Django `devices` app
- Django migrations
- MQTT catcher management command
- Mosquitto configuration
- Caddy configuration
- documentation set under `docs/`
- operator helper code

## Confirmed working

Unknown / not yet verified in this session.

## Existing but unverified

- Docker Compose stack starts successfully
- PostgreSQL is reachable from Django
- Django migrations apply cleanly
- Mosquitto accepts local MQTT messages
- `mqtt_catcher` consumes MQTT messages
- MQTT messages are stored in the database
- API returns latest known device state
- Caddy routes HTTP traffic correctly

## Known gaps

- No confirmed STM32 firmware implementation in current state
- No confirmed field-device message format
- No confirmed physical relay input wiring implementation
- No confirmed LTE communication test
- No farmer-facing dashboard
- No notification flow
- No remote-control flow

## Current technical goal

Prove the smallest backend vertical slice:

```text
manual MQTT test message
→ Mosquitto
→ Django MQTT catcher
→ PostgreSQL
→ API latest state response