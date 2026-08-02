4. State Machines

4.1 Purpose

This document describes the state-oriented behavior that exists in the completed BCM firmware and separates it from planned future state machines.

4.2 Implemented Vehicle State Machine

States

VEHICLE_STATE_OFF
VEHICLE_STATE_ACCESSORY
VEHICLE_STATE_RUNNING

State Diagram

                 Request ACCESSORY
        +------------------------------+
        |                              |
        v                              |
+---------------+              +---------------+
|      OFF      |              |   ACCESSORY   |
+---------------+              +---------------+
        ^                              |
        |                              |
        | Request OFF                  | Request RUNNING
        |                              v
        |                      +---------------+
        +----------------------|    RUNNING    |
           Request OFF         +---------------+
                                      |
                               Request ACCESSORY
                                      |
                                      v
                                ACCESSORY

The current implementation accepts explicit valid state requests rather than enforcing a complex ignition-key transition sequence.

Invalid values shall not become persistent valid states.

4.2.1 Initial State

After initialization:

Vehicle State = OFF

4.2.2 OFF State

Meaning:

Vehicle body functions are in the lowest operating state.

Essential monitoring may continue.

The state may transition to ACCESSORY or RUNNING if a valid command is accepted.

4.2.3 ACCESSORY State

Meaning:

Accessory operation is enabled.

The vehicle is not considered fully running.

The state may transition to OFF or RUNNING.

4.2.4 RUNNING State

Meaning:

Normal running-related behavior is enabled.

The state may transition to ACCESSORY or OFF.

4.2.5 Inputs

State transitions may be initiated by:

CAN command 0x100

Simulation command interface

The raw command is decoded outside the Vehicle State Manager.

4.3 Implemented Door State

The Door Manager maintains a logical state based on the front-left door input.

DOOR_STATE_CLOSED
DOOR_STATE_OPEN

GPIO input active
       |
       v
+---------------+
|     OPEN      |
+---------------+

GPIO input inactive
       |
       v
+---------------+
|    CLOSED     |
+---------------+

This is a direct logical state update rather than a timed multi-state actuator machine.

4.4 Implemented Battery State Machine

States

BATTERY_STATE_NORMAL
BATTERY_STATE_LOW
BATTERY_STATE_CRITICAL
BATTERY_STATE_OVER_VOLTAGE

Threshold Behavior

Critical:
Enter below 9000 mV
Recover at or above 9500 mV

Low:
Enter below 11000 mV
Recover at or above 11500 mV

Over-voltage:
Enter above 15500 mV
Recover at or below 15000 mV

Conceptual Diagram

                    Voltage > 15500
          +--------------------------------+
          |                                v
+----------------+                  +------------------+
|     NORMAL     |                  |  OVER_VOLTAGE    |
+----------------+                  +------------------+
   |          ^                         |
   |          |                         |
   |          | Voltage <= 15000        |
   | Voltage < 11000                    |
   v                                    |
+----------------+                      |
|      LOW       |----------------------+
+----------------+
   |          ^
   |          |
   |          | Voltage >= 9500 / 11500
   |
   | Voltage < 9000
   v
+----------------+
|    CRITICAL    |
+----------------+

The exact transition order follows the implementation, but the central design principle is hysteresis.

4.5 Interior Lighting Decision Logic

The current Lighting Manager is a decision-based control function rather than a multi-state exterior-lighting machine.

Conceptual behavior:

Door Open?
   |
   +-- No --> Interior Light OFF
   |
   +-- Yes
         |
     Battery Suitable?
         |
         +-- No --> Interior Light OFF
         |
         +-- Yes --> Interior Light ON

Vehicle state may also be considered according to the implemented logic.

The output request is sent through the GPIO driver.

4.6 Fault State Representation

Faults are represented as active bits rather than individual state-machine objects.

Source Condition Active
          |
          v
Set Fault Bit

Source Condition Cleared
          |
          v
Clear Fault Bit

Implemented fault bits:

Battery low

Battery critical

Battery over-voltage

Invalid vehicle state

4.7 Diagnostic State Representation

The Diagnostic Manager does not currently maintain default and extended UDS sessions.

Instead, every periodic cycle:

Reads the current fault mask.

Rebuilds the DTC mask.

Compares it with the previous DTC mask.

Logs applicable newly active DTCs.

Fault Mask
    |
    v
Build DTC Mask
    |
    v
Compare Previous Mask
    |
New 0 -> 1 transition?
    |
    +-- Yes --> Log Event
    |
    +-- No --> No New Event

4.8 CAN Processing State

The implemented CAN service does not maintain a formal INIT/ACTIVE/TIMEOUT/FAULT application state machine.

Its operational flow is:

CANTask Start
    |
Initialize / Start CAN
    |
Poll Receive
    |
Validate Frame
    |
Process Supported Command
    |
Transmit BCM Status Periodically
    |
Delay and Repeat

CAN timeout detection and bus-off recovery are planned extensions.

4.9 Invalid State Handling

Each state-oriented module shall use a defined default branch.

General pattern:

switch (state)
{
    case VALID_STATE_1:
        break;

    case VALID_STATE_2:
        break;

    default:
        state = SAFE_STATE;
        break;
}

The system should never continue indefinitely with an uncontrolled enumeration value.

4.10 FreeRTOS Integration

State and control functions execute from periodic tasks.

VehicleTask      -> VehicleStateManager
LightingTask     -> DoorManager + LightingManager
BatteryTask      -> BatteryMonitor
FaultTask        -> FaultManager
DiagnosticTask   -> DiagnosticManager
CANTask          -> CANService

The application modules remain non-blocking and return to the scheduler after each cycle.

4.11 Planned Future State Machines

The following designs are planned but not implemented in the current firmware.

BCM Operating Mode

Planned states:

INIT

ACTIVE

SLEEP

FAULT

Exterior Lighting

Planned states:

OFF

LOW_BEAM

HIGH_BEAM

Turn Indicators

Planned states:

OFF

ON

with timer-based flashing.

Hazard Warning

Planned behavior:

Hazard request overrides individual left/right requests.

Both outputs flash synchronously.

Door Lock Control

Planned states:

LOCKED

UNLOCKED

with pulse-based actuator commands.

Wiper Control

Planned states:

OFF

INTERMITTENT

LOW

HIGH

Diagnostic Sessions

Planned states:

DEFAULT

EXTENDED

CAN Health

Planned states:

INIT

ACTIVE

TIMEOUT

FAULT

These planned state machines should not be described as completed functionality in the README or resume.