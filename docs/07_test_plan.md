7. Test Plan

7.1 Purpose

This document defines the verification strategy for the completed BCM firmware.

It distinguishes:

Automated tests that exist and pass

Integration checks performed in Renode

Planned future tests for unimplemented features

7.2 Test Objectives

The completed test suite verifies:

Vehicle state behavior

Door input processing

Interior lighting logic

Battery thresholds and hysteresis

Fault activation and clearing

DTC mapping

Event logging

Ring-buffer behavior

CAN command processing

BCM status encoding

Driver-interface behavior

Invalid and boundary input handling

7.3 Test Levels

Level

Current status

Purpose

Unit testing

Implemented

Test modules on host PC

Module testing

Implemented through unit suites

Test complete module behavior

Integration testing

Partially implemented

Test interaction through mocks and Renode

Simulation testing

Implemented

Run target firmware in Renode

Hardware testing

Planned

Validate on physical STM32 and CAN hardware

HIL testing

Planned

Validate against external hardware/test system

7.4 Unit Test Framework

The project uses:

Ceedling
Unity
CMock

Hardware-facing dependencies are mocked.

Example:

LightingManager
      |
      v
Mock DoorManager / BatteryMonitor / GPIO Driver
      |
      v
Expected output assertion

The suite contains 153 passing tests.

7.5 Test Structure

Each test follows:

Arrange

Act

Assert

Recommended naming pattern:

test_<module>_should_<expected_result>_when_<condition>

Example:

void test_CANService_ShouldRequestOffStateForIgnitionCommand0(void);

7.6 As-Built Unit Test Areas

7.6.1 Vehicle State Manager

Tests shall verify:

Initial state is OFF

Valid state requests are accepted

OFF can be requested

ACCESSORY can be requested

RUNNING can be requested

Invalid values are rejected or safely handled

Getter returns the current state

7.6.2 Door Manager

Tests shall verify:

Door initializes to a defined state

GPIO inactive maps to CLOSED

GPIO active maps to OPEN

State getter returns the processed value

Driver interaction is correct

7.6.3 Lighting Manager

Tests shall verify:

Interior lamp initializes safely

Door-open condition requests the correct light state

Door-closed condition turns the lamp off

Low or critical battery condition inhibits lighting where required

GPIO output calls match the expected state

Repeated cycles remain deterministic

7.6.4 Battery Monitor

Tests shall verify:

ADC value is read correctly

Voltage is converted to millivolts

Normal voltage produces NORMAL state

Voltage below 11000 mV enters LOW

Voltage below 9000 mV enters CRITICAL

Voltage above 15500 mV enters OVER_VOLTAGE

Low state recovers at or above 11500 mV

Critical state recovers at or above 9500 mV

Over-voltage recovers at or below 15000 mV

Boundary values are handled correctly

Hysteresis prevents state chatter

7.6.5 Fault Manager

Tests shall verify:

Initialization clears all fault bits

Battery-low state sets bit 0

Battery-critical state sets bit 1

Battery-over-voltage state sets bit 2

Invalid vehicle state sets bit 3

Conditions clearing remove their active bits

Multiple simultaneous faults combine correctly

Fault mask getter returns the complete mask

Invalid fault IDs are handled safely

The implemented Fault Manager does not maintain separate stored faults, occurrence counters, or historical status.

7.6.6 Diagnostic Manager

Tests shall verify:

DTC mask initializes to zero

DTC 0x1001 follows the battery-low fault

DTC 0x1002 follows the battery-critical fault

DTC 0x1003 follows the battery-over-voltage fault

DTC 0x1004 follows the invalid-vehicle-state fault

DTCs clear when source faults clear

Multiple DTCs combine correctly

Invalid DTC queries are rejected

Newly active battery DTCs generate one event

Continuously active DTCs do not generate duplicate events every cycle

7.6.7 Event Logger and Ring Buffer

Tests shall verify:

Logger initializes empty

A new event is stored

Event values are preserved

Events are returned in expected order

Buffer count is correct

Buffer overwrites the oldest entry after 16 records

Total event count continues increasing

No dynamic allocation is required

Empty-buffer reads are handled safely

7.6.8 CAN Service

Tests shall verify:

RX 0x100

ID 0x100 is recognized

DLC 1 is required

Command 0 requests OFF

Command 1 requests ACCESSORY

Command 2 requests RUNNING

Unsupported command values do not cause uncontrolled state changes

Unsupported CAN IDs are ignored or rejected safely

Receive processing stops when no more frames are available

TX 0x200

CAN ID is 0x200

DLC is 8

Byte 0 encodes vehicle state

Byte 1 encodes door state

Bytes 2–3 encode battery voltage little-endian

Bytes 4–7 encode the active fault mask little-endian

The CAN driver transmit function receives the expected frame

7.6.9 Driver Tests

Driver-level host tests shall verify, where implemented:

Driver initialization behavior

Correct STM32 HAL function interactions through mocks/stubs

Receive return values

Transmit return values

Invalid pointers or inputs are handled according to the driver contract

Application modules remain independent of STM32 HAL

7.7 Regression Test Execution

Run:

ceedling test:all

Expected result:

153 tests
153 passed
0 failures
0 ignored

The exact count should be updated if new tests are added.

7.8 Test Reports

The project test runner generates evidence such as:

Console output

Text report

JSON summary

HTML report

Typical files:

ceedling_output.txt
test_summary.json
test_report.html

Generated reports may be retained selectively as project evidence while temporary build files remain excluded from Git.

7.9 Continuous Integration

GitHub Actions shall:

Check out the repository.

Install the required Ruby/Ceedling environment.

Run the unit tests.

Fail the workflow if any required test fails.

No test should be removed merely to make the pipeline pass.

7.10 Renode Integration Testing

The target ELF is executed in Renode.

Integration checks include:

CPU starts from the expected vector table

UART analyzer displays firmware checkpoints

FreeRTOS scheduler starts

Tasks continue executing

Simulation commands are accepted

Status output is produced

CAN initialization completes under the final virtual configuration

Firmware does not remain in Error_Handler()

Long-running simulation remains stable

7.11 CAN Debug Verification

The CAN issue was verified through:

UART checkpoints.

CPU program-counter inspection.

addr2line source mapping.

CAN register inspection.

Renode CAN peripheral configuration.

Re-running the firmware after changes.

Pass condition:

Firmware proceeds beyond CAN startup and continues normal scheduled execution.

7.12 FreeRTOS Stack Verification

The stack issue was addressed by increasing the Vehicle task stack.

Verification shall include:

Scheduler continues running

Vehicle task remains responsive

UART command processing works

No repeated reset or unexpected task termination occurs

Other tasks continue executing

Future measurable verification should include:

uxTaskGetStackHighWaterMark()

and a stack-overflow hook.

7.13 Python GUI Tests

The GUI shall be checked for:

Startup without exceptions

Renode connection behavior

Manual command controls

Classical CAN frame entry

Send button visibility in smaller window sizes

Communication-log updates

Automated test invocation

Clear/reset behavior

Error messages for invalid user input

The GUI is a test and demonstration tool, not part of the embedded target firmware.

7.14 Current Pass Criteria

The completed project milestone passes when:

All 153 host unit tests pass

STM32 target builds successfully

Renode loads the ELF

FreeRTOS tasks execute

UART simulation interface works

CAN startup does not trap the firmware in Error_Handler()

Python GUI communicates with the simulation

No unresolved critical defect prevents the demonstrated feature set

7.15 Current Out-of-Scope Tests

The following tests belong to planned features and shall not be reported as passing today:

Headlight low/high beam

Turn indicator timing

Hazard priority

Door lock actuator pulse

Wiper state transitions

CAN message timeouts

CAN bus-off recovery

Diagnostic session control

Read Data By Identifier

ECU reset service

Read/Clear DTC through CAN

Historical DTC storage

DTC occurrence counters

Physical CAN transceiver tests

Hardware-in-the-loop tests

ISO 26262 coverage targets

7.16 Planned Future Verification

CAN

Timeout at exact boundaries

Bus-off entry and recovery

Queue overflow

Interrupt-driven reception

Multiple ECU traffic

DBC conformance

Diagnostics

UDS positive and negative responses

Session timeout

Read and clear DTC

Persistent storage

Power-cycle behavior

RTOS

Task response-time measurement

Stack high-water mark

Priority inversion checks

Queue capacity and overload

Long-duration stress test

Watchdog supervision

Hardware

Physical STM32F407 board

CAN transceiver

Logic analyzer

Oscilloscope

Real switch inputs and output loads

SIL-to-HIL progression

7.17 Requirements Traceability

A lightweight traceability table should link every implemented requirement to one or more tests.

Example:

Requirement

Verification

Vehicle OFF/ACC/RUNNING

Vehicle State Manager tests

Door open/closed

Door Manager tests

Interior light

Lighting Manager tests

Battery hysteresis

Battery Monitor boundary tests

Fault mask

Fault Manager tests

DTC mapping

Diagnostic Manager tests

Ring buffer

Event Logger/Ring Buffer tests

CAN ignition command

CAN Service RX tests

BCM status frame

CAN Service TX tests

FreeRTOS execution

Renode integration test

GUI communication

GUI integration check

The table should be expanded when new functionality is added.