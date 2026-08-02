2. Automotive BCM ECU — Software Requirements

2.1 Document Status

This document contains:

Requirements implemented in the completed firmware.

Planned extensions that are not yet implemented.

The current implementation status in 01_project_overview.md takes priority whenever a planned requirement and the as-built firmware differ.

2.2 Project Purpose

The project shall demonstrate a simplified Body Control Module ECU using Embedded C, STM32F407, FreeRTOS, Classical CAN, host-based unit testing, Renode simulation, and a Python test GUI.

The project shall remain modular, hardware-independent where practical, and suitable for technical demonstration and interview discussion.

2.3 Implemented Functional Requirements

REQ-VEH-001 — Vehicle States

The firmware shall maintain one of the following vehicle states:

VEHICLE_STATE_OFF

VEHICLE_STATE_ACCESSORY

VEHICLE_STATE_RUNNING

REQ-VEH-002 — Vehicle State Transitions

The Vehicle State Manager shall validate requested state values.

Valid requests shall update the current state.

Invalid values shall be rejected or returned to a defined safe state according to the module implementation.

REQ-VEH-003 — Vehicle State Sources

Vehicle state requests shall be accepted through:

CAN ignition command 0x100

Simulation command input

REQ-DOOR-001 — Door State

The system shall monitor the front-left door state.

Supported states:

DOOR_STATE_CLOSED

DOOR_STATE_OPEN

REQ-DOOR-002 — Driver Abstraction

Door status shall be obtained through the GPIO driver abstraction rather than by direct register access from the application module.

REQ-LIGHT-001 — Interior Light Control

The Lighting Manager shall control the interior lamp based on:

Door state

Vehicle state

Battery state

REQ-LIGHT-002 — Hardware Independence

The application lighting logic shall not directly call STM32 GPIO registers.

Output control shall occur through the GPIO driver interface.

REQ-LIGHT-003 — Low-Voltage Protection

The interior lamp shall be inhibited when the battery condition is unsuitable for normal lighting operation according to the implemented logic.

REQ-BAT-001 — Voltage Sampling

The Battery Monitor shall read battery voltage through the ADC driver abstraction.

REQ-BAT-002 — Battery States

The monitor shall classify voltage into:

Normal

Low

Critical

Over-voltage

REQ-BAT-003 — Hysteresis

Separate entry and recovery thresholds shall be used to avoid repeated state changes near a voltage boundary.

Implemented thresholds:

State

Entry condition

Recovery condition

Critical

< 9000 mV

>= 9500 mV

Low

< 11000 mV

>= 11500 mV

Over-voltage

> 15500 mV

<= 15000 mV

REQ-FAULT-001 — Centralized Fault Storage

Active faults shall be stored in a single uint32_t fault mask.

REQ-FAULT-002 — Supported Faults

The implemented fault set shall include:

Battery low

Battery critical

Battery over-voltage

Invalid vehicle state

REQ-FAULT-003 — Level-Based Evaluation

The Fault Manager shall re-evaluate the current source conditions periodically and set or clear the corresponding fault bits.

REQ-DIAG-001 — DTC Mapping

Each active fault shall map to one DTC:

DTC

Code

Battery low

0x1001

Battery critical

0x1002

Battery over-voltage

0x1003

Invalid vehicle state

0x1004

REQ-DIAG-002 — Active DTC Mask

The Diagnostic Manager shall rebuild its active DTC mask from the Fault Manager state.

REQ-DIAG-003 — Volatile Storage

DTC state may be stored in RAM only for the current implementation.

REQ-DIAG-004 — Event-on-Activation

A diagnostic event shall be logged only when an applicable DTC changes from inactive to active.

REQ-EVENT-001 — Fixed-Size Log

Events shall be stored in a 16-entry fixed-size ring buffer.

REQ-EVENT-002 — No Dynamic Allocation

The event logger shall not require malloc() or free().

REQ-EVENT-003 — Full Buffer Behavior

When full, the ring buffer shall overwrite the oldest entry.

REQ-EVENT-004 — Lifetime Count

The event logger shall track the total number of events recorded even when older entries have been overwritten.

REQ-CAN-001 — CAN Type

The implemented communication interface shall use Classical CAN with 11-bit standard identifiers.

REQ-CAN-002 — Ignition Command

The BCM shall receive vehicle-state commands using:

CAN ID: 0x100
DLC: 1

REQ-CAN-003 — BCM Status

The BCM shall transmit status using:

CAN ID: 0x200
DLC: 8

REQ-CAN-004 — Driver Isolation

The CAN Service shall access CAN through the CAN driver abstraction.

REQ-CAN-005 — Invalid Input Handling

Unsupported CAN IDs, invalid DLC values, and out-of-range command values shall not cause uncontrolled state changes.

REQ-RTOS-001 — RTOS

The STM32 target shall use FreeRTOS.

REQ-RTOS-002 — Task Decomposition

The firmware shall use the following task structure:

Vehicle task

Lighting task

Battery task

Fault task

Diagnostic task

CAN task

REQ-RTOS-003 — Periodic Execution

Each task shall execute its module functions periodically and return control to the scheduler.

REQ-RTOS-004 — Non-Blocking Behavior

Application and service main functions shall avoid:

Infinite internal loops

Busy waits

Long blocking delays

REQ-RTOS-005 — Task Stack Capacity

Each task shall have sufficient stack for its call depth, local data, and communication activity.

The Vehicle task shall use the increased stack allocation required by the final integrated firmware.

REQ-ARCH-001 — Layered Architecture

The software shall separate:

Application modules

Service modules

Driver abstractions

RTOS integration

STM32 platform code

Simulation and tooling

REQ-ARCH-002 — HAL Isolation

Application and service modules shall not directly depend on STM32 peripheral registers.

REQ-ARCH-003 — Testability

Hardware-independent logic shall compile and run in host-based unit tests.

REQ-TEST-001 — Unit Test Framework

The project shall use:

Ceedling

Unity

CMock

REQ-TEST-002 — Hardware Mocking

Driver dependencies shall be replaced with CMock mocks during host testing.

REQ-TEST-003 — Regression Suite

All automated tests shall run after significant changes.

REQ-TEST-004 — Continuous Integration

GitHub Actions shall execute the host test suite on repository changes.

REQ-SIM-001 — Renode Execution

The STM32 firmware shall execute in Renode using the project machine and platform configuration.

REQ-SIM-002 — UART Simulation Interface

The simulation shall support command input and status/log output through UART.

REQ-SIM-003 — Python GUI

The project shall provide a Python GUI for:

Simulation control

Manual command input

Classical CAN frame entry

Communication logs

Automated test execution

REQ-SIM-004 — Simulation-Specific Configuration

Simulation-specific workarounds or peripheral setup shall be isolated so that application logic remains portable to real hardware.

2.4 Implemented Non-Functional Requirements

REQ-NF-001 — Embedded C Practices

The code shall use:

Fixed-width integer types where appropriate

Enumerations for states

Named constants

Private module state

Defined public APIs

Defensive validation

REQ-NF-002 — Static Memory Preference

The application and service layers shall avoid unnecessary dynamic memory.

REQ-NF-003 — Build Quality

The host test build should compile with warnings enabled.

REQ-NF-004 — Documentation Accuracy

Documentation shall distinguish completed implementation from planned features.

REQ-NF-005 — Scope Claims

The project shall not claim:

ISO 26262 certification

AUTOSAR compliance

Formal MISRA compliance

Production UDS

Production cybersecurity

Real-vehicle deployment readiness

unless separately implemented and verified.

2.5 Planned Functional Extensions

The following requirements are design targets and are not part of the completed firmware.

Planned Body Functions

Exterior headlight control

High-beam control

Turn indicators

Hazard warning

Door lock/unlock actuator control

Wiper control

Input debounce service

BCM sleep/wake management

Planned CAN Functions

Multiple receive and transmit messages

CAN communication timeout monitoring

CAN bus-off recovery

Driver switch message

Remote command message

Dedicated fault-status message

DBC generation

Planned Diagnostic Functions

Diagnostic sessions

Read Data By Identifier

Read DTC Information

Clear DTC Information

ECU Reset

ISO-TP support

Historical/confirmed DTC state

Occurrence counters

Non-volatile DTC storage

Planned Hardware Extensions

Physical STM32 board validation

Physical CAN transceiver

Multiple physical or simulated ECUs

Hardware-in-the-loop testing

Watchdog supervision

Bootloader concepts