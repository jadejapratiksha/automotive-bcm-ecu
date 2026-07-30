# Automotive BCM ECU - Software Requirements Specification

## 1. Project Purpose

This project implements a simplified Automotive Body Control Module (BCM) ECU using embedded C. The goal is to demonstrate ECU-style firmware architecture, state machines, diagnostics, CAN communication concepts, unit testing, and simulation support.

The BCM controls and monitors vehicle body functions such as ignition state, door lock status, lighting behavior, battery voltage, diagnostic faults, and CAN-based status reporting.

## 2. System Features

The BCM ECU shall provide the following major functions:

1. Vehicle State Management
   - Monitor and manage vehicle ignition state.
   - Support OFF, ACCESSORY, and RUNNING states.

2. Door Management
   - Monitor door open/closed status.
   - Control door lock and unlock state.

3. Lighting Management
   - Control vehicle headlights.
   - Control interior lighting based on vehicle and door conditions.

4. Battery Monitoring
   - Monitor simulated vehicle battery voltage.
   - Detect a low-battery condition.

5. Fault Management
   - Detect system faults.
   - Store active fault information.

6. Diagnostics
   - Provide diagnostic information about detected faults.
   - Support simplified Diagnostic Trouble Codes (DTCs).

7. CAN Communication
   - Receive simulated vehicle information through CAN messages.
   - Transmit BCM status through CAN messages.

8. Event Logging
   - Record important BCM events such as ignition changes,
     door events, lighting events, and detected faults.


     ## 3. Vehicle State Requirements

The BCM shall maintain the current operating state of the vehicle.

### 3.1 Vehicle States

The following vehicle states shall be supported:

- VEHICLE_OFF
- VEHICLE_ACCESSORY
- VEHICLE_RUNNING

### 3.2 State Transitions

The BCM shall support the following transitions:

OFF -> ACCESSORY
ACCESSORY -> OFF
ACCESSORY -> RUNNING
RUNNING -> ACCESSORY
RUNNING -> OFF

For the initial version of the project, vehicle state changes will be generated through the software simulator.

### 3.3 Vehicle State Behavior

When the vehicle is OFF:
- Normal vehicle operation shall be disabled.
- The BCM shall continue monitoring essential inputs such as door status.

When the vehicle is in ACCESSORY:
- Accessory-related BCM functions may operate.
- The engine shall be considered not running.

When the vehicle is RUNNING:
- Normal BCM functions shall be enabled.
- Battery monitoring and CAN status reporting shall be active.

## 4. Door Management Requirements

The BCM shall monitor and control the vehicle door state.

### 4.1 Door States

The following door states shall be supported:

- DOOR_CLOSED
- DOOR_OPEN

The following lock states shall be supported:

- DOOR_LOCKED
- DOOR_UNLOCKED

### 4.2 Door Inputs

For the initial software simulation, the following inputs shall be simulated:

- Door open command
- Door close command
- Lock command
- Unlock command

### 4.3 Door Behavior

When a door open command is received:
- The door state shall change to DOOR_OPEN.
- The event shall be recorded by the event logger.
- The interior lighting request shall be activated.

When a door close command is received:
- The door state shall change to DOOR_CLOSED.
- The event shall be recorded.

When a lock command is received:
- The lock state shall change to DOOR_LOCKED.

When an unlock command is received:
- The lock state shall change to DOOR_UNLOCKED.

### 4.4 Safety Behavior

The BCM shall not automatically lock the door while the simulated door state is DOOR_OPEN.

Automatic locking behavior may be added in a later project phase.

## 5. Lighting Management Requirements

The BCM shall control the simulated vehicle exterior and interior lighting.

### 5.1 Lighting States

The following headlight states shall be supported:

- HEADLIGHT_OFF
- HEADLIGHT_LOW_BEAM
- HEADLIGHT_HIGH_BEAM

The following interior light states shall be supported:

- INTERIOR_LIGHT_OFF
- INTERIOR_LIGHT_ON

### 5.2 Headlight Inputs

For the initial software simulation, the following commands shall be supported:

- Headlight OFF command
- Low beam command
- High beam command

### 5.3 Headlight Behavior

When the low beam command is received:
- The headlight state shall change to HEADLIGHT_LOW_BEAM.

When the high beam command is received:
- The headlight state shall change to HEADLIGHT_HIGH_BEAM.

When the headlight OFF command is received:
- The headlight state shall change to HEADLIGHT_OFF.

### 5.4 Interior Light Behavior

When the door state changes to DOOR_OPEN:
- The interior light shall turn ON.

When the door state changes to DOOR_CLOSED:
- The interior light shall turn OFF after the initial simplified behavior is processed.

More advanced timeout and fade behavior may be added in a later project phase.

## 6. Battery Monitoring Requirements

The BCM shall monitor the simulated vehicle battery voltage.

### 6.1 Battery Input

For the initial software simulation, battery voltage shall be provided as a simulated input.

The simulator shall allow the battery voltage to be changed so that normal and fault conditions can be tested.

### 6.2 Battery Operating Conditions

The initial project shall support the following battery conditions:

* BATTERY_NORMAL
* BATTERY_LOW

### 6.3 Low Battery Detection

The BCM shall detect a low-battery condition when the simulated battery voltage falls below the configured low-voltage threshold.

The threshold shall be defined as a configurable constant rather than being hard-coded throughout the application.

### 6.4 Battery Recovery

When battery voltage returns above the configured recovery threshold:

* The battery state shall return to BATTERY_NORMAL.
* The corresponding active fault shall be cleared according to the fault-management logic.

Separate detection and recovery thresholds may be used to prevent repeated switching between normal and fault states near the threshold.

## 7. Fault Management Requirements

The BCM shall provide centralized detection and management of system faults.

### 7.1 Fault States

A fault shall have one of the following states:

* FAULT_INACTIVE
* FAULT_ACTIVE

### 7.2 Initial Supported Faults

The initial implementation shall support at least:

* Low battery voltage
* CAN communication timeout
* Invalid door sensor state

Additional faults may be added in later project phases.

### 7.3 Fault Activation

When a fault condition is detected:

* The corresponding fault shall become active.
* The event shall be reported to the diagnostic manager.
* The event shall be recorded by the event logger.

### 7.4 Fault Clearing

When the fault condition is no longer present and its recovery conditions have been satisfied:

* The active fault may be cleared.
* The fault manager shall update the system fault status.

## 8. Diagnostic Requirements

The BCM shall provide simplified automotive diagnostic functionality.

### 8.1 Diagnostic Trouble Codes

Each supported diagnostic fault shall have a unique Diagnostic Trouble Code (DTC).

The initial implementation shall include:

* DTC_BATTERY_LOW
* DTC_CAN_TIMEOUT
* DTC_DOOR_SENSOR_INVALID

### 8.2 DTC Status

Each DTC shall maintain at least:

* DTC identifier
* Active/inactive status

Additional information such as occurrence count or timestamp may be added later.

### 8.3 Diagnostic Access

Diagnostic information shall be accessible to the software simulator so that active DTCs can be displayed visually.

The diagnostic design is intentionally simplified and is not intended to implement a complete production UDS diagnostic stack.

## 9. CAN Communication Requirements

The BCM shall support simulated Controller Area Network (CAN) communication.

### 9.1 CAN Communication

The CAN service shall provide interfaces for:

* Receiving CAN messages
* Processing supported CAN messages
* Transmitting BCM status messages
* Detecting selected communication timeouts

### 9.2 Initial CAN Messages

The project shall initially define CAN messages for:

* Vehicle/ignition state
* Door status
* Battery status
* Lighting status
* BCM fault/diagnostic status

The exact CAN identifiers, data length, signal positions, scaling, and transmission behavior shall be defined in:

`docs/05_can_database.md`

### 9.3 CAN Simulation

During PC-based development, CAN communication shall be simulated in software.

The simulation shall allow CAN messages to be generated, received, and displayed without requiring physical CAN hardware.

### 9.4 Future Hardware Integration

The application-level CAN logic shall be kept sufficiently independent from the hardware driver so that a future STM32 CAN peripheral implementation can be connected without rewriting the BCM application logic.

## 10. Event Logging Requirements

The BCM shall maintain a log of important system events.

### 10.1 Logged Events

The initial implementation shall support logging events including:

* Vehicle state changes
* Door open/close events
* Door lock/unlock events
* Lighting state changes
* Low battery detection
* Fault activation
* Fault clearing
* CAN timeout events

### 10.2 Event Storage

Events shall be stored using a fixed-size circular/ring buffer.

When the buffer becomes full:

* The oldest event shall be overwritten by the newest event.

The firmware shall avoid dynamic memory allocation for the event log.

### 10.3 Simulation

Logged events shall be accessible to the PC simulator for display and testing.

## 11. RTOS Requirements

The STM32 version of the BCM firmware shall use FreeRTOS.

### 11.1 Initial RTOS Responsibilities

The final embedded implementation is expected to separate major activities such as:

* BCM application/control processing
* CAN communication
* Diagnostic/fault processing
* Periodic monitoring
* Event/log processing

The exact task decomposition and priorities shall be defined during the software architecture phase.

### 11.2 Inter-Task Communication

Where required, FreeRTOS mechanisms may include:

* Queues
* Semaphores
* Mutexes
* Event groups
* Software timers

Each mechanism shall be selected based on the communication or synchronization requirement rather than used unnecessarily.

### 11.3 PC Development

FreeRTOS shall not be required for the initial host-based unit tests.

Hardware-independent application logic shall remain testable on the development computer independently of the STM32 hardware and RTOS.

## 12. Software Architecture Requirements

The BCM firmware shall use a modular layered architecture.

The software shall be separated into:

* Application modules
* Service modules
* RTOS integration
* Utility modules
* Hardware-specific STM32 code

Application logic shall not directly depend on STM32 HAL functions where practical.

Hardware-specific dependencies shall be isolated so that core BCM logic can be compiled and tested on a PC.

Detailed architecture shall be documented in:

`docs/03_software_architecture.md`

## 13. Unit Testing Requirements

Hardware-independent BCM modules shall be unit tested.

Ceedling and Unity shall be used for the host-based C unit-test environment.

Initial unit tests shall cover:

* Vehicle state transitions
* Door state behavior
* Door locking behavior
* Lighting behavior
* Battery threshold detection
* Fault activation and clearing
* DTC behavior
* Ring-buffer behavior
* CAN message processing where hardware-independent

Tests shall include both expected behavior and invalid/boundary conditions.

Detailed test cases shall be documented in:

`docs/07_test_plan.md`

## 14. Visual Simulation Requirements

The project shall include a PC-based visual simulation of the BCM.

The simulator shall allow the user to interact with the ECU model without requiring STM32 hardware.

### 14.1 Simulator Inputs

The simulator should provide controls for:

* Vehicle ignition/state
* Door open/close
* Door lock/unlock
* Headlight commands
* Battery voltage
* Selected CAN events/fault conditions

### 14.2 Simulator Outputs

The simulator should visually display:

* Current vehicle state
* Door status
* Door lock status
* Headlight status
* Interior light status
* Battery voltage/status
* Active faults
* Active DTCs

### 14.3 CAN Monitor

The simulation environment shall include a CAN-monitoring view or output showing simulated CAN traffic.

Where practical, the display shall include:

* Timestamp
* CAN identifier
* Data
* Message interpretation

### 14.4 GitHub Demonstration

Screenshots and/or short demonstrations of the completed simulator shall be stored with the project documentation so that project functionality can be understood without physical hardware.

## 15. STM32 Hardware Integration Requirements

The firmware architecture shall support later integration with an STM32 microcontroller.

STM32CubeMX/STM32CubeIDE shall be responsible for generating hardware-specific project files such as:

* Startup code
* STM32 HAL
* CMSIS
* Peripheral initialization
* FreeRTOS middleware configuration

Manually developed BCM application modules shall remain separate from generated hardware code where practical.

Future hardware interfaces may include:

* GPIO
* ADC
* CAN/FDCAN depending on selected STM32 device
* UART
* Hardware timers

The initial project shall not require physical STM32 hardware to develop and test hardware-independent BCM functionality.

## 16. Coding Requirements

The firmware shall be written primarily in C.

The implementation shall follow embedded-software practices including:

* Fixed-width integer types where appropriate
* Meaningful enumerations for states
* Named constants instead of unexplained magic numbers
* Clear separation between public interfaces and private implementation
* Limited global state
* No unnecessary dynamic memory allocation
* Defensive handling of invalid inputs
* Consistent naming and formatting

The project may follow MISRA-C-inspired practices; however, the project shall not claim formal MISRA compliance unless verified using an appropriate compliance process and toolchain.

## 17. CI/CD Requirements

The GitHub repository shall use automated continuous integration.

GitHub Actions shall eventually perform at least:

1. Checkout of the repository
2. Setup of the host test environment
3. Build of hardware-independent modules
4. Execution of unit tests
5. Failure of the workflow when required tests fail

Additional static-analysis checks may be added later.

## 18. Documentation Requirements

The project shall maintain the following engineering documentation:

* `01_project_overview.md` — project purpose and scope
* `02_requirements.md` — software requirements
* `03_software_architecture.md` — module and software architecture
* `04_state_machines.md` — state-machine definitions
* `05_can_database.md` — CAN messages and signals
* `06_diagnostics.md` — faults and DTC definitions
* `07_test_plan.md` — verification and unit-test strategy

Architecture diagrams, state diagrams, simulation screenshots, and test evidence shall be stored in the documentation/image directories as appropriate.

## 19. Project Scope Limitations

The initial portfolio project is a simplified educational BCM implementation.

The initial version does not claim to provide:

* Production automotive functional-safety compliance
* ISO 26262 certification
* AUTOSAR compliance
* Full UDS implementation
* Production cybersecurity
* Production bootloader functionality
* Production CAN network management
* Real vehicle deployment capability

These areas may be explored as future enhancements.

## 20. Future Enhancements

Potential future improvements include:

* Physical STM32 hardware deployment
* Real CAN transceiver integration
* Multiple simulated ECUs
* UDS diagnostic services
* Watchdog supervision
* Non-volatile DTC storage
* Bootloader concepts
* CAN database/DBC support
* CAN bus-off recovery
* Additional BCM functions such as indicators and hazard lights
* More advanced FreeRTOS task supervision
* Hardware-in-the-loop testing

```
```
