# Next Actions

## Current focus

Validate the physical hardware stack before committing to further firmware architecture.

## Immediate next actions

1. Validate the CP2102N USB-to-UART adapter independently.

   Success condition:

   * the host detects the adapter
   * the serial port can be opened
   * a TX-to-RX loopback test succeeds
   * the soldered adapter is confirmed working before connecting the modem

2. Validate the modem UART interface independently through the CP2102N.

   Success condition:

   * the modem powers and boots correctly
   * modem UART output is readable
   * basic AT commands receive valid responses
   * modem identity and firmware information can be queried
   * SIM presence or absence can be confirmed through AT commands

3. Obtain and install a suitable SIM before attempting cellular network validation.

4. After the standalone modem path is validated, define the smallest STM32-to-modem UART test.

   Success condition:

   * identify the STM32 UART peripheral and pins to use
   * transmit a basic AT command from the STM32
   * receive and inspect the modem response
   * keep MQTT, TLS, certificates, modem abstractions, retries, and production logging out of scope

## Deferred work

Do not add a dedicated logging task, modem driver architecture, MQTT publishing, certificate handling, identity loading, or real field-input integration until the hardware UART path has been validated.
