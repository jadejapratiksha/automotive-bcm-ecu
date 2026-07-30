# 7. Test Plan

## 7.1 Purpose

This document defines the verification and validation strategy for the Automotive Body Control Module (BCM) ECU project.

The objective of testing is to verify that the BCM software behaves according to the requirements defined in:

```text
docs/02_requirements.md
```

and follows the architecture and behavior defined in:

```text
docs/03_software_architecture.md
docs/04_state_machines.md
docs/05_can_database.md
docs/06_diagnostics.md
```

The project shall use multiple levels of testing:

* Unit testing.
* Module testing.
* State-machine testing.
* CAN communication testing.
* Diagnostic testing.
* Fault-injection testing.
* Integration testing.
* FreeRTOS behavior testing.
* Host-based system simulation.
* Regression testing.

The initial project shall be testable without physical STM32 hardware.

Hardware-specific testing may be added later when the firmware is ported to an STM32 target.

---

# 7.2 Test Objectives

The primary test objectives are to verify:

* BCM initialization behavior.
* BCM state transitions.
* Lighting control behavior.
* Turn indicator behavior.
* Hazard behavior.
* Door lock behavior.
* Wiper behavior.
* CAN receive processing.
* CAN transmit processing.
* CAN timeout detection.
* Diagnostic services.
* DTC handling.
* Fault recovery.
* Safe-state behavior.
* FreeRTOS task execution.
* Inter-task communication.
* Software robustness.
* Host simulation behavior.

Testing shall also verify that invalid or unexpected inputs do not cause uncontrolled system behavior.

---

# 7.3 Test Strategy

Testing shall be performed in multiple layers.

```text
                  +-----------------------+
                  |    System Testing     |
                  |   Host Simulation     |
                  +-----------+-----------+
                              |
                  +-----------v-----------+
                  | Integration Testing   |
                  +-----------+-----------+
                              |
                  +-----------v-----------+
                  |   Module Testing      |
                  +-----------+-----------+
                              |
                  +-----------v-----------+
                  |    Unit Testing       |
                  +-----------------------+
```

Each level verifies a different portion of the software.

---

# 7.4 Test Levels

The project shall use the following major test levels.

| Test Level           | Purpose                                                 |
| -------------------- | ------------------------------------------------------- |
| Unit Test            | Verify individual functions and modules                 |
| Module Test          | Verify complete module behavior                         |
| Integration Test     | Verify interaction between modules                      |
| System Test          | Verify complete BCM behavior                            |
| Fault Injection Test | Verify abnormal and failure behavior                    |
| Regression Test      | Ensure previous functionality still works after changes |

---

# 7.5 Unit Testing Framework

The project shall use:

```text
Ceedling
Unity
CMock
```

for embedded C unit testing.

These tools allow application modules to be tested on the development PC without STM32 hardware.

Example:

```text
BCM Module
   |
   +------> GPIO HAL
   |
   +------> Timer HAL
```

During unit testing:

```text
GPIO HAL  -> Mock GPIO
Timer HAL -> Mock Timer
```

This allows deterministic testing of application behavior.

---

# 7.6 Proposed Test Directory Structure

The test files may be organized as:

```text
test/
|
+-- test_lighting.c
+-- test_wiper.c
+-- test_door_lock.c
+-- test_bcm_manager.c
+-- test_can_service.c
+-- test_diagnostics.c
+-- test_fault_manager.c
+-- test_input_processing.c
```

Additional tests may be added as implementation grows.

---

# 7.7 Test Naming Convention

Test names should clearly describe:

```text
Condition + Expected Behavior
```

Example:

```c
void test_wiper_should_enter_low_mode_when_low_requested(void);
```

Another example:

```c
void test_hazard_should_override_left_turn_request(void);
```

Test names should communicate the test intent without requiring the implementation to be inspected.

---

# 7.8 Test Case Structure

Each test case should conceptually contain:

```text
1. Initial Condition

2. Input / Event

3. Function Execution

4. Expected Result
```

This corresponds to:

```text
Arrange
Act
Assert
```

Example:

```c
void test_lighting_should_enable_low_beam_when_switch_is_on(void)
{
    Lighting_Init();

    Lighting_SetMainLightRequest(true);

    Lighting_MainFunction();

    TEST_ASSERT_EQUAL(
        LIGHT_STATE_LOW_BEAM,
        Lighting_GetState()
    );
}
```

---

# 7.9 BCM Initialization Tests

The following initialization behavior shall be tested.

## TC-BCM-001 — Initial BCM State

Objective:

Verify that the BCM starts in the correct state.

Expected behavior:

```text
Power On
   |
   v
BCM_STATE_INIT
```

After successful initialization:

```text
BCM_STATE_ACTIVE
```

---

## TC-BCM-002 — Safe Initial Outputs

Verify that outputs start in safe states.

Expected:

```text
Low Beam       = OFF
High Beam      = OFF
Left Indicator = OFF
Right Indicator = OFF
Wiper          = OFF
Door Actuator  = INACTIVE
```

---

## TC-BCM-003 — Initialization Failure

Inject a simulated critical initialization failure.

Expected:

```text
BCM_STATE_INIT
      |
 Critical Failure
      |
      v
BCM_STATE_FAULT
```

---

# 7.10 BCM State Machine Tests

## TC-BCM-004 — ACTIVE to SLEEP

Conditions:

```text
Ignition = OFF
No active BCM request
No active diagnostic session
```

Expected:

```text
ACTIVE -> SLEEP
```

---

## TC-BCM-005 — SLEEP to ACTIVE

Inject wake condition:

```text
Ignition ON
```

Expected:

```text
SLEEP -> ACTIVE
```

---

## TC-BCM-006 — Door Wake Event

While BCM is sleeping, simulate a valid door or remote unlock event.

Expected:

```text
BCM wakes and transitions to ACTIVE.
```

---

## TC-BCM-007 — Critical Fault

Inject critical BCM fault.

Expected:

```text
ACTIVE -> FAULT
```

---

## TC-BCM-008 — Invalid BCM State

Force an invalid state value during a controlled unit test.

Expected:

* Invalid state is detected.
* Fault Manager receives an invalid-state fault.
* BCM transitions to a safe state.

---

# 7.11 Lighting Tests

## TC-LIGHT-001 — Lighting Initial State

Expected:

```text
LIGHT_STATE_OFF
```

---

## TC-LIGHT-002 — Low Beam ON

Input:

```text
Light_Switch = ON
```

Expected:

```text
LIGHT_STATE_LOW_BEAM

Low Beam = ON
```

---

## TC-LIGHT-003 — Low Beam OFF

Starting state:

```text
LOW_BEAM
```

Input:

```text
Light_Switch = OFF
```

Expected:

```text
LIGHT_STATE_OFF
```

---

## TC-LIGHT-004 — High Beam ON

Inputs:

```text
Light_Switch       = ON
High_Beam_Request  = ON
```

Expected:

```text
LIGHT_STATE_HIGH_BEAM
```

---

## TC-LIGHT-005 — High Beam OFF

Starting state:

```text
HIGH_BEAM
```

Input:

```text
High_Beam_Request = OFF
```

Expected:

```text
HIGH_BEAM -> LOW_BEAM
```

---

## TC-LIGHT-006 — Main Light Switch OFF During High Beam

Inputs:

```text
Current State = HIGH_BEAM
Light Switch  = OFF
```

Expected:

```text
LIGHT_STATE_OFF
```

and all headlamp outputs return to OFF.

---

# 7.12 Turn Indicator Tests

## TC-IND-001 — Left Indicator Request

Input:

```text
Left_Turn_Request = ON
```

Expected:

```text
Left Indicator  = FLASHING
Right Indicator = OFF
```

---

## TC-IND-002 — Right Indicator Request

Expected:

```text
Right Indicator = FLASHING
Left Indicator  = OFF
```

---

## TC-IND-003 — Flash Timing

Verify indicator output toggles at the defined flash interval.

Initial flash interval:

```text
500 ms
```

Expected sequence:

```text
ON
500 ms
OFF
500 ms
ON
```

---

## TC-IND-004 — Request Removed

While left indicator is active:

```text
Left_Turn_Request = OFF
```

Expected:

```text
Left Indicator = OFF
```

---

# 7.13 Hazard Tests

## TC-HAZ-001 — Hazard Activation

Input:

```text
Hazard_Request = ON
```

Expected:

```text
Left Indicator  = FLASHING
Right Indicator = FLASHING
```

---

## TC-HAZ-002 — Hazard Synchronization

Verify both indicators change output state simultaneously.

Expected:

```text
LEFT ON  + RIGHT ON

then

LEFT OFF + RIGHT OFF
```

---

## TC-HAZ-003 — Hazard Priority Over Left Turn

Inputs:

```text
Hazard_Request    = ON
Left_Turn_Request = ON
```

Expected:

```text
Both indicators flash.
```

---

## TC-HAZ-004 — Hazard Priority Over Right Turn

Inputs:

```text
Hazard_Request     = ON
Right_Turn_Request = ON
```

Expected:

```text
Both indicators flash.
```

---

## TC-HAZ-005 — Hazard Deactivation

Starting state:

```text
Hazard active
```

Input:

```text
Hazard_Request = OFF
```

Expected:

The BCM resumes processing normal left/right requests.

---

# 7.14 Door Lock Tests

## TC-LOCK-001 — Initial Door State

Verify the configured initial lock state.

For the initial project:

```text
UNLOCKED
```

may be used as the startup logical state unless changed during implementation.

---

## TC-LOCK-002 — Lock Request

Starting state:

```text
UNLOCKED
```

Input:

```text
Lock_Request = ON
```

Expected:

```text
UNLOCKED -> LOCKED
```

---

## TC-LOCK-003 — Unlock Request

Starting state:

```text
LOCKED
```

Input:

```text
Unlock_Request = ON
```

Expected:

```text
LOCKED -> UNLOCKED
```

---

## TC-LOCK-004 — Lock Actuator Pulse

Verify the actuator request is active only for the configured period.

Expected:

```text
Lock Command ON
      |
Command Duration
      |
      v
Lock Command OFF
```

The output shall not remain permanently energized.

---

## TC-LOCK-005 — Repeated Lock Request

Starting state:

```text
LOCKED
```

Input:

```text
Lock_Request = ON
```

Expected:

* State remains LOCKED.
* No invalid transition occurs.

---

# 7.15 Wiper Tests

## TC-WIPER-001 — Initial Wiper State

Expected:

```text
WIPER_STATE_OFF
```

---

## TC-WIPER-002 — Intermittent Mode

Input:

```text
Wiper_Mode = INTERMITTENT
```

Expected:

```text
OFF -> INTERMITTENT
```

---

## TC-WIPER-003 — Low Mode

Input:

```text
Wiper_Mode = LOW
```

Expected:

```text
WIPER_STATE_LOW
```

---

## TC-WIPER-004 — High Mode

Input:

```text
Wiper_Mode = HIGH
```

Expected:

```text
WIPER_STATE_HIGH
```

---

## TC-WIPER-005 — Return to OFF

Starting state:

```text
HIGH
```

Input:

```text
Wiper_Mode = OFF
```

Expected:

```text
HIGH -> OFF
```

---

## TC-WIPER-006 — Low to High

Expected:

```text
LOW -> HIGH
```

when high-speed mode is requested.

---

## TC-WIPER-007 — High to Low

Expected:

```text
HIGH -> LOW
```

when low-speed mode is selected.

---

# 7.16 Intermittent Wiper Tests

## TC-WIPER-INT-001 — Initial WAIT State

When intermittent mode starts:

Expected:

```text
WIPER_INT_WAIT
```

---

## TC-WIPER-INT-002 — Interval Expiration

After the intermittent interval expires:

Expected:

```text
WAIT -> SWEEP
```

---

## TC-WIPER-INT-003 — Sweep Completion

After simulated sweep completion:

Expected:

```text
SWEEP -> WAIT
```

---

## TC-WIPER-INT-004 — No Blocking

Verify intermittent operation does not prevent other BCM functionality from executing.

For example, while the wiper is waiting:

```text
Hazard request
```

shall still be processed normally.

---

# 7.17 Input Processing Tests

Input-processing tests shall verify:

* Raw digital input conversion.
* Input validation.
* Debounce behavior.
* Input state change detection.
* Default state initialization.

---

## TC-INPUT-001 — Stable Input

Apply stable active input.

Expected:

```text
Processed Input = ACTIVE
```

---

## TC-INPUT-002 — Switch Bounce

Simulate:

```text
0
1
0
1
1
0
1
```

within the debounce interval.

Expected:

The processed input shall not repeatedly toggle.

---

## TC-INPUT-003 — Debounced Activation

After input remains active for the required debounce period:

Expected:

```text
Processed Input changes to ACTIVE.
```

---

# 7.18 CAN Receive Tests

CAN receive testing shall verify all defined RX messages.

Messages:

```text
0x100 VEHICLE_STATUS

0x110 DRIVER_SWITCH_STATUS

0x120 REMOTE_COMMAND

0x700 DIAG_REQUEST
```

---

## TC-CAN-RX-001 — VEHICLE_STATUS Decode

Inject:

```text
CAN ID = 0x100
```

with known input values.

Verify:

* Ignition status.
* Engine status.
* Moving status.
* Vehicle speed.
* Battery voltage.

---

## TC-CAN-RX-002 — DRIVER_SWITCH_STATUS Decode

Inject a known frame.

Verify:

* Light switch.
* High beam.
* Left turn.
* Right turn.
* Hazard.
* Lock.
* Unlock.
* Wiper mode.

---

## TC-CAN-RX-003 — Wiper Decode

Inject:

```text
Byte 1 lower bits = 10 binary
```

Expected:

```text
WIPER_MODE_LOW
```

---

## TC-CAN-RX-004 — Unknown CAN ID

Inject an undefined CAN identifier.

Expected:

* Message ignored safely or reported according to the final CAN design.
* No memory corruption.
* No uncontrolled application state change.

---

# 7.19 CAN DLC Validation Tests

## TC-CAN-DLC-001 — Valid DLC

Input:

```text
DLC = 8
```

Expected:

Message is accepted.

---

## TC-CAN-DLC-002 — Invalid DLC

For a message requiring DLC 8, inject:

```text
DLC = 4
```

Expected:

* Message is rejected.
* `CAN_INVALID_DLC` fault is reported.
* Application data is not updated with incomplete data.

---

# 7.20 CAN Timeout Tests

## TC-CAN-TIMEOUT-001 — VEHICLE_STATUS Timeout

Stop sending:

```text
CAN ID 0x100
```

for more than:

```text
100 ms
```

Expected:

* Timeout detected.
* DTC `0x010101` becomes active.
* Vehicle-state values move to defined safe defaults.

---

## TC-CAN-TIMEOUT-002 — DRIVER_SWITCH_STATUS Timeout

Stop sending:

```text
CAN ID 0x110
```

Expected:

* Timeout detected.
* DTC `0x010102` active.
* Driver requests default inactive.

---

## TC-CAN-TIMEOUT-003 — Timeout Recovery

After timeout, inject a valid message.

Expected:

```text
Communication state returns to ACTIVE.
```

Active fault clears according to Fault Manager policy.

Stored fault may remain.

---

# 7.21 CAN Transmission Tests

The following BCM TX messages shall be tested:

```text
0x200 BCM_LIGHTING_STATUS

0x210 BCM_BODY_STATUS

0x220 BCM_FAULT_STATUS

0x708 DIAG_RESPONSE
```

---

## TC-CAN-TX-001 — Lighting Status

Set:

```text
Low Beam = ON
```

Verify:

```text
CAN ID = 0x200

Byte 0 Bit 0 = 1
```

---

## TC-CAN-TX-002 — Indicator Status

When indicator output is ON:

Expected corresponding CAN status bit:

```text
1
```

When output toggles OFF:

```text
0
```

---

## TC-CAN-TX-003 — Body Status

Verify:

* Vehicle lock state.
* Wiper state.
* BCM active status.
* BCM sleep status.
* BCM fault status.

---

## TC-CAN-TX-004 — Reserved Bits

Verify all reserved transmit bits are:

```text
0
```

---

# 7.22 CAN Period Tests

Verify periodic messages are transmitted at approximately the expected periods.

| CAN ID | Expected Period |
| ------ | --------------: |
| 0x200  |          100 ms |
| 0x210  |          100 ms |
| 0x220  |          500 ms |

Tolerance shall be defined during FreeRTOS integration.

---

# 7.23 Diagnostic Tests

The following services shall be tested:

```text
0x10 Diagnostic Session Control

0x11 ECU Reset

0x14 Clear Diagnostic Information

0x19 Read DTC Information

0x22 Read Data By Identifier
```

---

# 7.24 Diagnostic Session Tests

## TC-DIAG-001 — Initial Session

After initialization:

Expected:

```text
DIAG_SESSION_DEFAULT
```

---

## TC-DIAG-002 — Enter Extended Session

Request:

```text
10 03
```

Expected response:

```text
50 03
```

Expected state:

```text
DIAG_SESSION_EXTENDED
```

---

## TC-DIAG-003 — Return to Default Session

Request:

```text
10 01
```

Expected:

```text
DIAG_SESSION_DEFAULT
```

---

## TC-DIAG-004 — Session Timeout

Enter EXTENDED session.

Do not send diagnostic requests for:

```text
5000 ms
```

Expected:

```text
EXTENDED -> DEFAULT
```

---

# 7.25 Read DID Tests

## TC-DID-001 — Read BCM State

Request:

```text
22 01 00
```

Expected:

```text
62 01 00 XX
```

---

## TC-DID-002 — Read Lighting State

Request:

```text
22 01 01
```

Expected valid lighting state response.

---

## TC-DID-003 — Read Wiper State

Request:

```text
22 01 02
```

Verify returned state equals current wiper state.

---

## TC-DID-004 — Read Door Lock State

Request:

```text
22 01 03
```

Verify response matches lock state.

---

## TC-DID-005 — Read Fault Count

Request:

```text
22 01 04
```

Verify returned count equals Fault Manager active fault count.

---

## TC-DID-006 — Unsupported DID

Request:

```text
22 12 34
```

Expected:

```text
7F 22 31
```

---

# 7.26 Unsupported Diagnostic Service Test

## TC-DIAG-005

Request:

```text
99 00
```

Expected:

```text
7F 99 11
```

where:

```text
0x11 = Service Not Supported
```

---

# 7.27 Diagnostic Message Length Test

## TC-DIAG-006

Send incomplete request:

```text
22 F1
```

Expected:

```text
7F 22 13
```

where:

```text
0x13 = Incorrect Message Length or Invalid Format
```

---

# 7.28 ECU Reset Tests

## TC-DIAG-RESET-001 — Reset in Default Session

Request:

```text
11 01
```

while in DEFAULT session.

Expected:

Negative response because reset is restricted.

---

## TC-DIAG-RESET-002 — Reset in Extended Session

Sequence:

```text
10 03

11 01
```

Expected:

```text
51 01
```

followed by simulated ECU reinitialization.

---

## TC-DIAG-RESET-003 — Reset Application State

Before reset:

```text
Wiper = HIGH
Lights = ON
```

After reset:

Expected initialization defaults are restored.

---

# 7.29 Read DTC Tests

## TC-DTC-001 — No Faults

With no active or stored faults:

Request:

```text
19 02
```

Expected response shall indicate no applicable fault.

---

## TC-DTC-002 — Active CAN Timeout DTC

Inject `VEHICLE_STATUS` timeout.

Expected:

```text
DTC 0x010101
Active = TRUE
```

---

## TC-DTC-003 — Stored Fault

Recover communication.

Expected:

```text
Active = FALSE
Stored = TRUE
```

---

# 7.30 Clear DTC Tests

## TC-DTC-CLEAR-001 — Clear in Default Session

Request:

```text
14 FF FF FF
```

Expected:

Negative response.

---

## TC-DTC-CLEAR-002 — Clear in Extended Session

Sequence:

```text
10 03

14 FF FF FF
```

Expected:

Stored DTCs are cleared.

---

## TC-DTC-CLEAR-003 — Fault Still Present

Create active communication timeout.

Clear DTC.

If fault condition remains:

Expected:

```text
Fault becomes active again during subsequent monitoring.
```

---

# 7.31 Fault Manager Tests

The Fault Manager shall be tested independently.

---

## TC-FAULT-001 — Set Fault

Call:

```c
FaultManager_SetFault(DTC);
```

Expected:

```text
Active = TRUE
Stored = TRUE
```

---

## TC-FAULT-002 — Clear Active Fault

Expected:

```text
Active = FALSE
Stored = TRUE
```

---

## TC-FAULT-003 — Clear Stored Fault

After clear request:

Expected:

```text
Active = FALSE
Stored = FALSE
```

unless the fault condition remains present.

---

## TC-FAULT-004 — Occurrence Counter

Cause the same fault repeatedly.

Verify occurrence count increments according to the implemented policy.

---

## TC-FAULT-005 — Active Fault Count

Inject multiple faults.

Verify:

```text
FaultManager_GetActiveFaultCount()
```

returns the correct number.

---

# 7.32 Fault Injection Testing

Fault injection is an important part of the BCM project.

The host simulator shall allow faults to be intentionally created.

Possible injected conditions include:

* CAN timeout.
* Invalid CAN DLC.
* Invalid CAN ID.
* Invalid application state.
* Simulated GPIO failure.
* Timer failure.
* CAN bus-off condition.
* Corrupted diagnostic request.

---

# 7.33 GPIO Fault Injection

Simulate GPIO write failure.

Expected:

* Fault Manager receives GPIO output fault.
* Affected feature transitions to safe behavior.
* Diagnostic system reports the fault.

---

# 7.34 Timer Fault Injection

Simulate invalid timer behavior.

Expected:

* Timer fault reported.
* Time-dependent features do not enter uncontrolled behavior.

---

# 7.35 CAN Bus-Off Simulation

Simulate:

```text
CAN_STATE_FAULT
```

or bus-off condition.

Expected:

* DTC set.
* Application uses safe default CAN values where required.
* Recovery behavior is controlled.

---

# 7.36 Invalid State Injection

Force invalid values into state variables during controlled tests.

Modules to test:

* BCM.
* Lighting.
* Wiper.
* Door lock.

Expected:

* Invalid state detected.
* Fault reported.
* Module returns to safe state.

---

# 7.37 Integration Testing

Integration tests verify interactions between multiple software components.

Examples:

```text
CAN Service
     +
Lighting Module
     +
CAN Status Transmission
```

or:

```text
CAN Timeout
     +
Fault Manager
     +
Diagnostic Service
```

---

# 7.38 Integration Test — CAN to Lighting

## TC-INT-001

Inject:

```text
CAN ID 0x110

Light_Switch = 1
```

Expected flow:

```text
CAN Frame
   |
   v
CAN Service Decode
   |
   v
Driver Switch Data
   |
   v
Lighting State Machine
   |
   v
Low Beam ON
   |
   v
BCM_LIGHTING_STATUS
```

---

# 7.39 Integration Test — Hazard

## TC-INT-002

Inject CAN message containing:

```text
Hazard_Request = 1
```

Expected:

* CAN Service decodes signal.
* Lighting logic activates hazard mode.
* Both indicators flash.
* CAN TX status represents the correct lamp state.

---

# 7.40 Integration Test — CAN Timeout to Diagnostics

## TC-INT-003

Stop sending:

```text
VEHICLE_STATUS
```

Expected chain:

```text
CAN Timeout Monitor
       |
       v
Fault Manager
       |
       v
DTC 0x010101
       |
       v
Diagnostic Read DTC
       |
       v
Tester sees fault
```

---

# 7.41 Integration Test — Clear Fault

## TC-INT-004

1. Inject fault.
2. Enter extended diagnostic session.
3. Send clear DTC command.
4. Remove original fault condition.
5. Read DTC information.

Expected:

Stored DTC no longer present.

---

# 7.42 FreeRTOS Testing

The final BCM application shall run using FreeRTOS.

Testing shall verify:

* Task creation.
* Task periods.
* Task priorities.
* Inter-task communication.
* Queue operation.
* No uncontrolled blocking.
* No task starvation.
* Stable long-duration execution.

---

# 7.43 Proposed Task Tests

Possible tasks include:

```text
Input Task

Body Control Task

CAN Task

Diagnostic Task
```

Test each task independently where practical and verify integrated scheduling behavior.

---

# 7.44 Task Period Tests

Example target periods:

| Task              | Period |
| ----------------- | -----: |
| Input Task        |  10 ms |
| Body Control Task |  10 ms |
| CAN Task          |  10 ms |
| Diagnostic Task   | 100 ms |

Actual periods shall match the final software architecture.

---

# 7.45 FreeRTOS Queue Tests

If CAN reception uses a FreeRTOS queue:

Test:

```text
CAN Frame
    |
Queue Send
    |
Queue Receive
    |
CAN Processing
```

Verify:

* Frame contents remain correct.
* Multiple frames are handled correctly.
* Queue-full condition is handled safely.

---

# 7.46 Diagnostic Queue Tests

If a diagnostic request queue is implemented:

Verify:

* Diagnostic CAN frames are passed to the Diagnostic Task.
* Non-diagnostic messages are not placed in the diagnostic queue.
* Queue overflow is detected or handled safely.

---

# 7.47 Task Starvation Test

Run the BCM under high simulated activity.

Examples:

* Frequent CAN frames.
* Continuous lighting activity.
* Wipers active.
* Repeated diagnostic requests.

Verify all required tasks continue executing.

---

# 7.48 Long-Duration Stability Test

Run the host simulation for an extended number of scheduler cycles.

Verify:

* No crash.
* No task lockup.
* No increasing memory usage.
* No corrupted state.
* CAN processing remains functional.
* Diagnostics remain responsive.

The simulation duration may initially be based on accelerated virtual time rather than real elapsed hours.

---

# 7.49 Host Simulation Testing

The host simulator shall provide a way to exercise the BCM without hardware.

Example:

```text
+------------------------------------------+
|              BCM Simulator               |
|------------------------------------------|
| Ignition:            ON                  |
| Light Switch:        ON                  |
| High Beam:           OFF                 |
| Left Turn:           ON                  |
| Right Turn:          OFF                 |
| Hazard:              OFF                 |
| Wiper:               LOW                 |
| Door State:          LOCKED              |
|                                          |
| BCM State:           ACTIVE              |
| Active Faults:       0                   |
+------------------------------------------+
```

The exact visual interface may be console-based initially.

---

# 7.50 Simulation Input Testing

The simulator shall support changing inputs such as:

```text
Ignition

Lighting Switch

High Beam

Left Turn

Right Turn

Hazard

Wiper Mode

Lock Request

Unlock Request
```

The BCM output shall update according to the software logic.

---

# 7.51 CAN Simulation Tests

A Python or C-based tool may inject CAN frames.

Example command:

```text
python send_can.py 0x110 05 00 00 00 00 00 00 00
```

Expected BCM behavior shall be compared against the CAN database.

---

# 7.52 Diagnostic Simulation Tests

A simulated diagnostic tester shall send commands such as:

```text
22 01 00
```

and verify response:

```text
62 01 00 XX
```

The tool may later decode the response into human-readable text.

---

# 7.53 End-to-End Test Scenario 1 — Vehicle Startup

## TC-E2E-001

Sequence:

```text
1. Start BCM.

2. BCM enters INIT.

3. Initialization succeeds.

4. BCM enters ACTIVE.

5. Ignition CAN signal becomes ON.

6. Light switch becomes ON.

7. Wiper mode becomes LOW.
```

Expected:

```text
BCM State   = ACTIVE
Low Beam    = ON
Wiper       = LOW
No Faults
```

---

# 7.54 End-to-End Test Scenario 2 — Turn Signal

## TC-E2E-002

Sequence:

```text
Ignition ON

Left Turn Request ON
```

Expected:

```text
Left indicator flashes every defined interval.

Right indicator remains OFF.
```

---

# 7.55 End-to-End Test Scenario 3 — Hazard Override

## TC-E2E-003

Sequence:

```text
Left Turn Request = ON

then

Hazard Request = ON
```

Expected:

```text
Both indicators flash together.
```

---

# 7.56 End-to-End Test Scenario 4 — Communication Failure

## TC-E2E-004

Sequence:

```text
1. BCM operating normally.

2. Stop VEHICLE_STATUS messages.

3. Wait >100 ms.

4. BCM detects timeout.

5. Diagnostic tester requests DTC information.
```

Expected:

```text
DTC 0x010101 reported.
```

---

# 7.57 End-to-End Test Scenario 5 — Diagnostic Clear

## TC-E2E-005

Sequence:

```text
1. Create fault.

2. Remove fault condition.

3. Fault becomes stored.

4. Enter extended session.

5. Send clear DTC command.

6. Read DTC information.
```

Expected:

Stored DTC is cleared.

---

# 7.58 End-to-End Test Scenario 6 — Remote Unlock

## TC-E2E-006

Inject:

```text
CAN ID 0x120

Remote_Unlock = 1
```

Expected:

```text
Door Lock State -> UNLOCKED
```

---

# 7.59 End-to-End Test Scenario 7 — Wiper Mode Change

Sequence:

```text
OFF
 |
 v
INTERMITTENT
 |
 v
LOW
 |
 v
HIGH
 |
 v
OFF
```

Expected:

The Wiper State Machine correctly follows each requested mode.

---

# 7.60 Requirements Traceability

Each software requirement should eventually be linked to one or more test cases.

Example:

| Requirement                 | Test                   |
| --------------------------- | ---------------------- |
| Lighting low beam operation | TC-LIGHT-002           |
| High beam operation         | TC-LIGHT-004           |
| Hazard priority             | TC-HAZ-003, TC-HAZ-004 |
| Wiper low mode              | TC-WIPER-003           |
| CAN timeout monitoring      | TC-CAN-TIMEOUT-001     |
| Diagnostic session control  | TC-DIAG-002            |
| Clear DTC                   | TC-DTC-CLEAR-002       |

A complete requirement-to-test traceability matrix may be added once implementation begins.

---

# 7.61 Test Pass Criteria

A test shall be considered PASS when:

* Actual result matches expected result.
* No unexpected fault occurs.
* No memory or runtime error occurs.
* No unrelated application state is modified.
* Output values remain within valid ranges.

---

# 7.62 Test Fail Criteria

A test shall be considered FAIL when:

* Expected state transition does not occur.
* Output differs from expected value.
* Incorrect CAN data is generated.
* Incorrect diagnostic response is returned.
* Required fault is not detected.
* Unexpected fault is generated.
* Software crashes.
* Task becomes unresponsive.
* Data corruption occurs.

---

# 7.63 Regression Testing

All automated unit tests shall be executed after significant code changes.

Example:

```text
Source Code Change
       |
       v
Run Unit Tests
       |
       v
Run Integration Tests
       |
       v
Review Results
```

Existing tests shall not be removed simply to allow new code to pass.

Tests should be updated only when the intended software behavior or requirement changes.

---

# 7.64 Continuous Integration

The project should eventually use GitHub Actions to automatically run tests.

This follows the same workflow used in the earlier embedded CI practice project.

Conceptually:

```text
Git Push
   |
   v
GitHub Actions
   |
   v
Install Dependencies
   |
   v
Run Ceedling Tests
   |
   v
PASS / FAIL
```

The CI pipeline may later run:

```text
ceedling test:all
```

and reject changes when automated tests fail.

---

# 7.65 Code Coverage

Unit-test code coverage may be measured where practical.

Useful metrics include:

* Function coverage.
* Statement coverage.
* Branch coverage.

State-machine modules should receive particular attention to branch coverage because each transition represents different behavior.

The project does not initially require a specific automotive production coverage standard.

The goal is meaningful verification rather than artificially maximizing a coverage percentage.

---

# 7.66 State Transition Coverage

For each state machine, tests shall attempt to exercise every valid transition.

Example for wipers:

```text
OFF -> INTERMITTENT
OFF -> LOW
OFF -> HIGH

INTERMITTENT -> OFF
INTERMITTENT -> LOW
INTERMITTENT -> HIGH

LOW -> OFF
LOW -> INTERMITTENT
LOW -> HIGH

HIGH -> OFF
HIGH -> INTERMITTENT
HIGH -> LOW
```

This provides more useful coverage than testing only individual states.

---

# 7.67 Boundary Testing

Boundary conditions shall be tested where applicable.

Examples:

CAN timeout:

```text
99 ms
100 ms
101 ms
```

Indicator timer:

```text
499 ms
500 ms
501 ms
```

Diagnostic timeout:

```text
4999 ms
5000 ms
5001 ms
```

These tests verify correct comparison logic at timing boundaries.

---

# 7.68 Robustness Testing

The project shall include robustness tests such as:

* Null pointer input where APIs permit pointers.
* Invalid enumeration values.
* Maximum fault count.
* Unsupported CAN IDs.
* Invalid diagnostic requests.
* Repeated CAN frames.
* Rapid switch transitions.
* Queue-full conditions.
* Repeated diagnostic requests.

---

# 7.69 Static Analysis

Static analysis may later be added to identify:

* Uninitialized variables.
* Dead code.
* Type conversion issues.
* Potential buffer overflows.
* Unreachable states.
* Suspicious pointer use.

Possible tools may include compiler warnings and open-source static analyzers.

At minimum, the firmware shall be compiled with warnings enabled.

---

# 7.70 Compiler Warning Policy

The project should aim to compile with:

```text
zero compiler warnings
```

Warnings shall not simply be ignored unless there is a documented reason.

Typical warning flags on host GCC may include:

```text
-Wall
-Wextra
-Wpedantic
```

Additional flags may be introduced as development progresses.

---

# 7.71 Memory Testing

The project shall avoid unnecessary dynamic allocation.

Testing shall verify:

* Static buffers are not exceeded.
* CAN payload processing respects DLC.
* Diagnostic response buffers are not exceeded.
* Fault table capacity is respected.
* Queue elements use valid sizes.

---

# 7.72 Test Data

Test data should include:

* Normal values.
* Minimum values.
* Maximum values.
* Invalid values.
* Timing boundary values.
* Repeated values.
* Unexpected state combinations.

For example, vehicle speed:

```text
0 km/h
1 km/h
100 km/h
250 km/h
```

and any raw value outside the defined logical range shall be handled according to CAN validation rules.

---

# 7.73 Test Evidence

Test execution results should be retained where useful.

Possible output directory:

```text
results/
```

Example:

```text
results/
|
+-- unit_test_results.txt
+-- integration_test_results.txt
+-- can_test_results.txt
+-- diagnostic_test_results.txt
+-- coverage/
```

Generated build artifacts should normally remain excluded from Git unless they are intentionally retained as project evidence.

---

# 7.74 Automated Test Script

A Python script may later execute the complete host-based test flow.

Example:

```text
python scripts/run_tests.py
```

Possible behavior:

```text
1. Build firmware.

2. Run unit tests.

3. Run simulation tests.

4. Run CAN test scenarios.

5. Run diagnostic scenarios.

6. Report PASS / FAIL.
```

Example output:

```text
=================================
 AUTOMOTIVE BCM TEST REPORT
=================================

Unit Tests        : PASS
Lighting Tests    : PASS
Wiper Tests       : PASS
CAN Tests         : PASS
Diagnostic Tests  : PASS
Fault Tests       : PASS

=================================
 ALL TESTS PASSED
=================================
```

---

# 7.75 Future Hardware-in-the-Loop Testing

When STM32 hardware is introduced, additional tests may include:

* Physical GPIO testing.
* CAN transceiver communication.
* Real CAN bus traffic.
* Hardware timers.
* Real FreeRTOS timing.
* Physical switch inputs.
* LED or load outputs.
* Logic analyzer measurements.
* Oscilloscope measurements.

This can extend the project from:

```text
SIL
Software-in-the-Loop
```

toward:

```text
HIL
Hardware-in-the-Loop
```

testing.

---

# 7.76 Software-in-the-Loop Testing

The initial project primarily uses Software-in-the-Loop testing.

Conceptually:

```text
+-----------------------------------------+
|                PC                       |
|                                         |
|   Test Tool / Python Simulator          |
|                |                        |
|                v                        |
|        Simulated CAN / Inputs           |
|                |                        |
|                v                        |
|            BCM Firmware                 |
|                |                        |
|                v                        |
|        Simulated HAL Outputs            |
|                                         |
+-----------------------------------------+
```

This allows most application logic to be developed before purchasing or connecting hardware.

---

# 7.77 Test Completion Criteria

The initial software milestone shall be considered successfully tested when:

* All planned unit tests pass.
* All major state transitions are exercised.
* CAN encode/decode tests pass.
* CAN timeout detection passes.
* Diagnostic services return expected responses.
* DTC set/read/clear behavior passes.
* Fault-injection tests pass.
* FreeRTOS tasks execute without starvation.
* Host simulation completes the defined end-to-end scenarios.
* No unresolved critical software defects remain.

---

# 7.78 Initial Test Matrix

| Area              | Test Type           | Priority |
| ----------------- | ------------------- | -------- |
| BCM State Machine | Unit / Integration  | High     |
| Lighting          | Unit / Integration  | High     |
| Indicators        | Unit / Timing       | High     |
| Hazard            | Unit / Integration  | High     |
| Door Locks        | Unit / Integration  | High     |
| Wipers            | Unit / Timing       | High     |
| CAN Decode        | Unit                | High     |
| CAN Encode        | Unit                | High     |
| CAN Timeout       | Integration / Fault | High     |
| Diagnostics       | Unit / Integration  | High     |
| Fault Manager     | Unit                | High     |
| FreeRTOS          | Integration         | High     |
| Input Debounce    | Unit                | Medium   |
| Host Simulation   | System              | High     |
| STM32 Hardware    | Hardware            | Future   |

---

# 7.79 Final Verification Flow

The complete verification strategy is:

```text
Requirements
     |
     v
Unit Tests
     |
     v
Module Tests
     |
     v
Integration Tests
     |
     v
CAN + Diagnostic Tests
     |
     v
Fault Injection
     |
     v
FreeRTOS Integration
     |
     v
Host System Simulation
     |
     v
Regression Testing
     |
     v
Future STM32 Hardware Testing
```

This provides a progressive path from individual C functions to a complete simulated automotive ECU.

---

# 7.80 Summary

The Automotive BCM test strategy uses automated and simulation-based testing to verify the firmware before physical hardware is required.

The project shall test:

* BCM state management.
* Lighting.
* Turn signals.
* Hazard operation.
* Door locking.
* Wipers.
* Input processing.
* CAN communication.
* CAN timeout handling.
* Diagnostics.
* DTC handling.
* Fault recovery.
* FreeRTOS scheduling.
* Inter-task communication.
* Full host-based system behavior.

The primary unit-test framework shall use:

```text
Ceedling
Unity
CMock
```

while Python and host simulation may be used for higher-level CAN, diagnostic, and end-to-end verification.

The test architecture is designed so that the project can later progress from host-based Software-in-the-Loop testing to STM32 hardware and eventually Hardware-in-the-Loop style testing.
