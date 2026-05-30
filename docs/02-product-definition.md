## 2. Problems

Greek farmers with irrigated fields often use on-site three-phase water pumps controlled locally through a manual start/stop circuit. Before irrigation begins, the farmer prepares the field installation, then physically starts the pump and verifies that the system is operating correctly. After leaving the site, the farmer loses visibility into the actual system status. He may not know whether the pump is still running, whether power has been interrupted, or whether irrigation is continuing as expected.

### 2.1 Pump stoppage from electrical problems

The pump can stop unexpectedly because of an electrical problem. When this happens without remote visibility, the failure may go unnoticed until someone drives by or returns to the field.

This creates downtime and uncertainty about how much watering actually happened.

### 2.2 Irrigation failures while the pump is running

The pump may continue running while the irrigation system is not working correctly. Failures such as burst pipes, disconnected lines, major leaks, blocked filters, closed valves, or insufficient pressure can prevent water from reaching the intended area.

This creates uncertainty about whether watering actually happened correctly, while also risking wasted water, uneven irrigation, equipment stress, or crop damage.

### 2.3 Wasted trips to control and view the system

The pump is controlled locally, and the farmer must be physically near the pump, button, or electrical panel to start or stop irrigation. The same applies when the farmer wants to verify the current system status or check whether a power outage has been resolved.

This creates unnecessary trips while on site and unnecessary driving after leaving the field.

## 3. Proposed Solution

The proposed solution is a remote monitoring and control system for agricultural irrigation pumps.

The system adds a practical visibility and control layer on top of existing field pump installations. It should allow the farmer to view the current pump and irrigation status, receive alerts when important failures are detected, and control the pump remotely when appropriate.

The goal is not to replace the existing irrigation setup, but to make it observable, controllable, and easier to trust from a distance.

### 3.1 Remote pump status

The system should make the irrigation system key states remotely visible to the farmer. This includes mains power state, pump relay status, and water pressure.

### 3.2 Notifications

The system should notify the user when the system behaves unexpectedly. 
Notifications include:
- pump state change
- mains state change
- pressure loss while pump is running

### 3.3 Remote start and stop control

The system should allow the farmer to start or stop the pump remotely.

### 3.4 Simple farmer-facing interface

The farmer should be able to view and control the system through a simple interface.

The interface should focus on practical field questions:

- Is the pump running?
- Is irrigation pressure normal?
- Has something failed?
- Can I start or stop the pump?
- When did the current irrigation session start?
- How long has it been running?

The interface should avoid unnecessary technical complexity.

## 4. Trust, Safety, and Reliability Principles

### 4.1 Fail-safe behavior
The system must not create dangerous or unexpected pump behavior. If communication, software, or remote control fails, the installation should remain safe and predictable.

### 4.2 Local control remains operational
The existing local start/stop circuit should remain usable. The remote system should add control and visibility without making the farmer dependent on the app.

### 4.3 Clear state reporting
The system should clearly distinguish between known states and unknown states.

### 4.4 Notification reliability
Important notifications should be delivered as reliably as possible, especially for pump stoppage, mains state change, and pressure loss while the pump is running.

### 4.5 Safe remote control
Remote start and stop should include safeguards against accidental operation, unclear state, or unsafe conditions.

### 4.6 Electrical safety
The system must respect the existing electrical installation and avoid unsafe modifications to the three-phase pump control circuit.

### 4.7 Farmer trust
The farmer should be able to understand what the system knows, what it does not know, and why it raised an alert.

## 5. MVP Scope

The MVP is the first usable version of the product. Its purpose is to prove the core value of remote visibility for an existing irrigation pump installation.

The MVP should focus on one field, one pump, and one farmer-facing interface.

### 5.1 Included in MVP

The MVP includes:

- remote visibility of mains power state
- remote visibility of pump relay state
- simple interface showing the current system state