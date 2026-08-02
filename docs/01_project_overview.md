1. Project Overview

1.1 Purpose

This document introduces the Automotive Body Control Module (BCM) ECU project, summarizes the completed functionality, explains the repository structure, and identifies which features are implemented versus planned.

This is the primary entry point for recruiters, reviewers, and contributors.

1.2 Project Summary

This project implements a simplified automotive Body Control Module in Embedded C for an STM32F407 microcontroller running FreeRTOS.

The project demonstrates:

Layered embedded-software architecture

Hardware abstraction

FreeRTOS task decomposition

CAN communication

State-machine-based control

Battery monitoring with hysteresis

Centralized fault and diagnostic handling

Fixed-size event logging

Host-based unit testing using Ceedling, Unity, and CMock

STM32 firmware execution in Renode

A Python GUI for simulation control and communication monitoring

The project is educational and portfolio-oriented. It is not a certified production automotive ECU and does not claim ISO 26262, AUTOSAR, production UDS, or cybersecurity compliance.

1.3 As-Built Functionality

The completed firmware provides the following functions.

Vehicle State Management

The vehicle state manager supports:

VEHICLE_STATE_OFF

VEHICLE_STATE_ACCESSORY

VEHICLE_STATE_RUNNING

State changes can be requested through the CAN service and through the simulation interface.

Door Monitoring

The front-left door state is read through the GPIO driver and represented as:

DOOR_STATE_CLOSED

DOOR_STATE_OPEN

Interior Lighting

The interior light is controlled according to:

Door state

Vehicle state

Battery condition

The lighting logic remains hardware-independent and controls the output through the GPIO driver abstraction.

Battery Monitoring

Battery voltage is sampled through the ADC driver.

The battery monitor supports:

BATTERY_STATE_NORMAL

BATTERY_STATE_LOW

BATTERY_STATE_CRITICAL

BATTERY_STATE_OVER_VOLTAGE

Separate entry and recovery thresholds provide hysteresis and prevent repeated state switching near a voltage boundary.

Fault Management

The Fault Manager stores active conditions in a uint32_t bitmask.

Implemented faults:

Low battery

Critical battery

Over-voltage battery

Invalid vehicle state

Diagnostics

The Diagnostic Manager maps the active fault bits to four DTCs:

DTC_BATTERY_LOW — 0x1001

DTC_BATTERY_CRITICAL — 0x1002

DTC_BATTERY_OVER_VOLTAGE — 0x1003

DTC_INVALID_VEHICLE_STATE — 0x1004

DTCs are RAM-resident and clear when their source fault clears.

Event Logging

Newly active diagnostic conditions are recorded in a fixed 16-entry ring buffer.

The event logger:

Uses no dynamic memory

Overwrites the oldest event when full

Tracks the total lifetime number of events separately

CAN Communication

The implemented Classical CAN interface uses 11-bit standard identifiers.

Implemented messages:

RX 0x100 — ignition/state command

TX 0x200 — BCM status

The complete implemented layout is documented in 05_can_database.md.

FreeRTOS Scheduling

The firmware uses six periodic FreeRTOS tasks:

Task

Primary responsibility

Period

VehicleTask

Vehicle state and simulation command processing

50 ms

LightingTask

Door and interior-light processing

20 ms

BatteryTask

Battery sampling and classification

100 ms

FaultTask

Active fault evaluation

100 ms

DiagnosticTask

DTC mirroring and event generation

100 ms

CANTask

CAN initialization, reception, processing, and status transmission

10 ms

Unit Testing

The project contains 153 passing host-based unit tests using:

Ceedling

Unity

CMock

The tests run without STM32 hardware by mocking the driver interfaces.

Renode Simulation and Python GUI

The final project includes:

STM32F407 firmware execution in Renode

UART-based simulation commands

Renode scripts for normal and SocketCAN-oriented execution

A Python GUI for control, command transmission, and communication logging

Automated test execution and HTML/JSON reports

1.4 High-Level Architecture

+------------------------------------------------------+
|                  Python GUI / Test Tools             |
|       Commands, CAN frames, logs, automated tests    |
+----------------------------+-------------------------+
                             |
                  UART / Virtual CAN transport
                             |
+----------------------------v-------------------------+
|                    STM32F407 Firmware                |
|                                                      |
|  Application: Vehicle, Door, Lighting, Battery      |
|  Services: CAN, Faults, Diagnostics, Event Logging  |
|  Drivers: GPIO, ADC, CAN                            |
|  RTOS: Six periodic FreeRTOS tasks                  |
+----------------------------+-------------------------+
                             |
+----------------------------v-------------------------+
|                  STM32 HAL / Renode Model            |
+------------------------------------------------------+

1.5 Repository Structure

automotive-bcm-ecu/
|
+-- firmware/
|   +-- App/
|   +-- Services/
|   +-- Drivers/
|   +-- RTOS/
|   +-- Utils/
|
+-- platform/stm32f407/
|   +-- Core/
|   +-- Drivers/
|   +-- Middlewares/
|   +-- Debug/
|
+-- tests/
|   +-- unit/
|   +-- support/
|
+-- simulation/renode/
|   +-- bcm.resc
|   +-- bcm_socketcan.resc
|   +-- platform description files
|
+-- tools/
|   +-- bcm_gui/
|   +-- test_runner/
|
+-- docs/
+-- .github/workflows/
+-- project.yml
+-- README.md

Generated build outputs, IDE metadata, logs, and temporary artifacts should remain excluded from the public repository through .gitignore.

1.6 Implemented vs. Planned Scope

Area

Status

Vehicle state: OFF / ACCESSORY / RUNNING

Implemented

Front-left door sensing

Implemented

Interior lighting

Implemented

Battery monitoring with hysteresis

Implemented

Centralized fault bitmask

Implemented

Four DTCs mirroring active faults

Implemented

Diagnostic event logging

Implemented

CAN RX 0x100 and TX 0x200

Implemented

Six FreeRTOS tasks

Implemented

153 host-based unit tests

Implemented

GitHub Actions test execution

Implemented

Renode firmware execution

Implemented

Python GUI and communication log

Implemented

Headlights and high beam

Planned

Turn indicators and hazards

Planned

Door lock/unlock actuation

Planned

Wiper control

Planned

CAN timeout DTC

Planned

Full eight-message CAN database

Planned

UDS diagnostic sessions and services

Planned

Non-volatile DTC storage

Planned

Physical CAN transceiver integration

Planned

Production functional-safety compliance

Out of scope

1.7 Important Debugging Outcomes

Renode CAN Startup Issue

The firmware initially printed early UART checkpoints but did not reach normal application execution.

Investigation showed that:

GPIO initialization completed.

CAN initialization/startup did not complete under the initial Renode setup.

The CPU entered Error_Handler().

Address-to-source mapping and CAN register inspection identified HAL_CAN_Start() as the failure path.

The simulated CAN controller did not initially have the same bus conditions as a physical CAN network.

The final design moved CAN startup and processing into CANTask, ensured the required RTOS objects were ready, and updated the Renode CAN configuration for virtual-bus operation.

This was a simulator integration issue, not an application-state-machine defect.

FreeRTOS Task Stack Issue

One real-time task required more stack than its original configuration.

The task also handled simulation command parsing, UART operations, local variables, and nested calls in addition to vehicle processing. Its stack was increased to:

512 words × 4 bytes = 2048 bytes

Other lightweight tasks remained at approximately:

128 words × 4 bytes = 512 bytes

Stable task execution was verified after the change.

A production improvement would be to enable stack-overflow hooks and measure stack high-water marks for every task.

1.8 Development Philosophy

Keep application logic independent from STM32 HAL.

Use fixed-width integer types and explicit enumerations.

Avoid unnecessary dynamic allocation.

Use non-blocking periodic functions.

Centralize fault management.

Keep state machines deterministic.

Mock hardware dependencies during host testing.

Validate simulator-specific behavior separately from hardware-independent logic.

Document planned features separately from completed features.

1.9 Document Map

Document

Purpose

01_project_overview.md

As-built project summary

02_requirements.md

Implemented requirements plus clearly identified planned requirements

03_software_architecture.md

Actual software layers, modules, tasks, and data flow

04_state_machines.md

Implemented state behavior and planned future state machines

05_can_database.md

Implemented two-message CAN protocol plus planned extensions

06_diagnostics.md

As-built fault, DTC, and event-logging design

07_test_plan.md

Actual test strategy and planned future verification

1.10 Running the Unit Tests

gem install ceedling
ceedling test:all

The complete host test suite runs without a physical STM32 board.