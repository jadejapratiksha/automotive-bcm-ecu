3. Software Architecture

3.1 Purpose

This document describes the architecture of the completed Automotive BCM ECU firmware.

It focuses on the software that exists in the repository and separates planned extensions from the as-built design.

3.2 Architectural Goals

The architecture is designed to:

Keep application behavior independent from STM32 hardware.

Support host-based unit testing.

Run on STM32F407 under FreeRTOS.

Run in Renode without rewriting the application logic.

Isolate simulation-specific code.

Provide clear module responsibilities.

Support deterministic periodic execution.

Centralize faults, diagnostics, and communication behavior.

3.3 As-Built Layered Architecture

+----------------------------------------------------------+
|                    Application Layer                     |
|----------------------------------------------------------|
| VehicleStateManager | DoorManager | LightingManager      |
| BatteryMonitor                                           |
+-------------------------------+--------------------------+
                                |
+-------------------------------v--------------------------+
|                      Service Layer                       |
|----------------------------------------------------------|
| CANService | FaultManager | DiagnosticManager            |
| EventLogger                                              |
+-------------------------------+--------------------------+
                                |
+-------------------------------v--------------------------+
|                  Project Driver Abstraction              |
|----------------------------------------------------------|
| GPIO Driver | ADC Driver | CAN Driver                    |
+-------------------------------+--------------------------+
                                |
+-------------------------------v--------------------------+
|              STM32 HAL / CMSIS-RTOS2 Integration         |
|----------------------------------------------------------|
| GPIO | ADC | CAN | UART | FreeRTOS | Startup             |
+-------------------------------+--------------------------+
                                |
+-------------------------------v--------------------------+
|             STM32F407 Hardware or Renode Model           |
+----------------------------------------------------------+

The phrase “project driver abstraction” is used to avoid confusion with the STM32 HAL library.

3.4 Application Layer

3.4.1 Vehicle State Manager

Responsibilities:

Store the current vehicle state.

Initialize to a defined state.

Accept valid state changes.

Reject or safely handle invalid values.

Expose the current state to other modules.

Supported states:

OFF

ACCESSORY

RUNNING

3.4.2 Door Manager

Responsibilities:

Read the front-left door through the GPIO driver.

Convert the physical/simulated input into a logical state.

Expose the door state to the Lighting Manager and CAN status encoder.

Supported states:

CLOSED

OPEN

3.4.3 Lighting Manager

Responsibilities:

Determine the required interior-lamp state.

Use vehicle state, door state, and battery condition.

Control the output through the GPIO driver.

Avoid direct STM32 HAL access.

The current implementation controls the interior lamp only.

Headlamps, turn indicators, hazards, and fade timing are planned extensions.

3.4.4 Battery Monitor

Responsibilities:

Read battery voltage through the ADC driver.

Convert the sampled value into millivolts.

Classify the current condition.

Apply entry and recovery thresholds.

Expose battery state and voltage to other modules.

The hysteresis design prevents chatter near threshold boundaries.

3.5 Service Layer

3.5.1 CAN Service

Responsibilities:

Receive frames through the CAN driver.

Validate supported identifiers and DLC.

Decode ignition commands.

Request vehicle-state transitions.

Encode BCM status.

Periodically transmit BCM status.

Implemented receive frame:

0x100 ignition command

Implemented transmit frame:

0x200 BCM status

3.5.2 Fault Manager

Responsibilities:

Maintain a centralized active-fault bitmask.

Re-evaluate fault source conditions periodically.

Set and clear fault bits.

Provide fault queries.

Provide the fault mask to the CAN service and Diagnostic Manager.

3.5.3 Diagnostic Manager

Responsibilities:

Map faults to DTCs.

Build the current DTC mask.

Detect newly active DTCs.

Create diagnostic events.

Expose DTC status to tests and other software.

The current implementation does not provide UDS request/response services.

3.5.4 Event Logger

Responsibilities:

Store events in a fixed 16-entry ring buffer.

Record event ID and associated value.

Overwrite the oldest record when full.

Track lifetime event count.

3.6 Driver Layer

3.6.1 GPIO Driver

Provides hardware-independent APIs used by:

Door Manager

Lighting Manager

The STM32 implementation maps these calls to STM32 HAL GPIO functions.

The host tests replace the GPIO driver with CMock mocks.

3.6.2 ADC Driver

Provides battery-voltage access.

The target implementation uses the STM32 ADC path where supported.

The Renode environment may not model every ADC behavior exactly, so the simulation interface can supply deterministic values for application testing.

3.6.3 CAN Driver

Provides:

Initialization

Start

Receive

Transmit

Driver-level validation/status

The CAN Service does not directly access CAN registers.

3.7 FreeRTOS Architecture

The firmware uses a periodic multitasking model.

+------------------------------------------------------+
|                 FreeRTOS Scheduler                   |
+----------+----------+----------+----------+-----------+
           |          |          |          |
           v          v          v          v
       Vehicle     Lighting    Battery     CAN
        Task        Task        Task       Task

           +----------+----------+
           |                     |
           v                     v
        Fault Task         Diagnostic Task

3.7.1 Task Configuration

Task

Main work

Period

VehicleTask

Vehicle state and simulation input

50 ms

LightingTask

Door and interior-light processing

20 ms

BatteryTask

Battery monitor

100 ms

FaultTask

Fault evaluation

100 ms

DiagnosticTask

DTC and event processing

100 ms

CANTask

CAN RX/TX and service processing

10 ms

The modules themselves remain independent of FreeRTOS where practical. This allows the same main functions to be called directly from host unit tests.

3.7.2 Task Stack Design

The integrated firmware exposed an insufficient-stack issue in the task that performed vehicle and simulation command processing.

Contributing factors included:

UART input handling

Command parsing

Local buffers and variables

Nested function calls

Additional simulation code not present in isolated unit tests

The final Vehicle task stack was increased to:

512 stack words × 4 bytes = 2048 bytes

Lightweight tasks remained at approximately:

128 stack words × 4 bytes = 512 bytes

Recommended future improvements:

Enable configCHECK_FOR_STACK_OVERFLOW.

Implement vApplicationStackOverflowHook().

Use uxTaskGetStackHighWaterMark() for each task.

Record measured margins in the documentation.

3.8 Initialization Sequence

MCU Reset
   |
   v
HAL and Clock Initialization
   |
   v
GPIO / UART / ADC / CAN Peripheral Setup
   |
   v
Application and Service Initialization
   |
   v
Create RTOS Objects and Tasks
   |
   v
Start FreeRTOS Scheduler
   |
   v
CANTask Starts CAN Operation
   |
   v
Normal Periodic Execution

CAN operation is handled in CANTask so that task context and communication objects are ready before continuous CAN processing begins.

3.9 CAN Data Flow

Receive Path

Renode CAN / Physical CAN
          |
          v
      CAN Driver
          |
          v
      CAN Service
          |
 Validate ID, DLC, command
          |
          v
VehicleStateManager_SetState()

Transmit Path

Vehicle / Door / Battery / Fault State
                   |
                   v
              CAN Service
                   |
          Encode BCM_STATUS
                   |
                   v
              CAN Driver
                   |
                   v
          CAN Bus / Simulator

3.10 Diagnostic Data Flow

Battery Monitor / Vehicle State
             |
             v
        Fault Manager
             |
             v
      Diagnostic Manager
             |
       0 -> 1 transition
             |
             v
         Event Logger

Fault detection and DTC reporting are intentionally separated.

3.11 Simulation Architecture

+--------------------+
|   Python GUI       |
|--------------------|
| Manual commands    |
| CAN frame entry    |
| Logs               |
| Test execution     |
+---------+----------+
          |
          | UART / Virtual transport
          v
+---------+----------+
| Renode STM32F407   |
|--------------------|
| Firmware ELF       |
| UART model         |
| CAN model          |
| GPIO model         |
+--------------------+

Simulation-specific code is guarded or isolated so that the application and service modules remain portable.

3.12 Renode CAN Startup Issue

Symptom

The firmware printed early UART status such as GPIO initialization success, but normal execution did not continue.

Investigation

The debugging process included:

Adding UART execution checkpoints.

Checking the final message printed before failure.

Mapping the stuck CPU address to source using addr2line.

Inspecting the CAN peripheral registers.

Confirming execution entered Error_Handler().

Root Cause

HAL_CAN_Start() expects the CAN controller to leave initialization mode after the bus and controller satisfy the required conditions.

The initial Renode environment did not automatically reproduce the complete physical CAN environment, including a connected virtual bus/peer and appropriate peripheral behavior.

The CAN controller therefore remained in or returned to an initialization-related state and the HAL timeout path called Error_Handler().

Why It Was Simulator-Specific

On physical hardware, CAN startup depends on:

Correct CAN clock configuration

Correct RX/TX pin alternate functions

A CAN transceiver

Proper bus wiring and termination

A valid recessive bus condition

In Renode, these physical conditions must be represented through the peripheral model and virtual CAN connections. A missing or incomplete simulation connection can block behavior even when the application code is logically correct.

Resolution

The final integration:

Started CAN from the CAN task after scheduler startup.

Ensured the queue and task environment were ready.

Updated Renode CAN configuration.

Added virtual CAN hub/SocketCAN-oriented scripts where appropriate.

Kept the workaround outside the hardware-independent application logic.

3.13 Testability

Every application and service module is compiled for host testing.

Example:

BatteryMonitor
     |
     v
Mock ADC Driver
     |
     v
Known voltage input
     |
     v
Expected state assertion

The same pattern is used for GPIO and CAN dependencies.

3.14 Planned Architectural Extensions

The following are not part of the completed design:

Exterior-lighting state machine

Turn signals and hazards

Door lock actuator logic

Wiper state machine

CAN timeout monitor

CAN bus-off recovery

Diagnostic session state machine

UDS and ISO-TP

Non-volatile DTC storage

Watchdog supervision

Bootloader

Physical multi-ECU CAN network