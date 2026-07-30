# 4. State Machines

## 4.1 Purpose

This document defines the major software state machines used by the Automotive Body Control Module (BCM) ECU.

State machines are used where system behavior depends on both current inputs and the previous operating state.

The BCM project uses state machines for:

* BCM operating mode.
* Exterior lighting.
* Turn indicators and hazard lamps.
* Door locking.
* Windshield wipers.
* Diagnostic sessions.
* Fault-related safe behavior.

The implementation shall use explicit states and clearly defined state transitions so that behavior is deterministic, testable, and easy to understand.

---

# 4.2 State Machine Design Principles

The following principles shall be followed when implementing BCM state machines:

* States shall be represented using enumerated types.
* State transitions shall be based on clearly defined conditions.
* State variables shall not be modified from multiple unrelated modules.
* Hardware access shall not occur directly inside transition logic.
* Blocking delays shall not be used for timing-dependent states.
* Time-based transitions shall use system timing information or FreeRTOS timing services.
* Invalid or unexpected states shall transition to a defined safe state.
* State transitions shall be testable using unit tests.
* Each state machine shall provide an initialization function.
* State machines shall execute periodically from the appropriate FreeRTOS task or application function.

Typical state-machine implementation:

```c
typedef enum
{
    STATE_IDLE = 0,
    STATE_ACTIVE,
    STATE_FAULT
} Example_State_t;

static Example_State_t current_state;

void Example_Init(void)
{
    current_state = STATE_IDLE;
}

void Example_MainFunction(void)
{
    switch (current_state)
    {
        case STATE_IDLE:
            break;

        case STATE_ACTIVE:
            break;

        case STATE_FAULT:
            break;

        default:
            current_state = STATE_IDLE;
            break;
    }
}
```

---

# 4.3 BCM Operating State Machine

The BCM Operating State Machine represents the overall operating mode of the ECU.

The initial BCM states are:

```text
BCM_STATE_INIT
BCM_STATE_ACTIVE
BCM_STATE_SLEEP
BCM_STATE_FAULT
```

## 4.3.1 BCM State Definitions

| State  | Description                                                                  |
| ------ | ---------------------------------------------------------------------------- |
| INIT   | BCM software and services are being initialized                              |
| ACTIVE | Normal BCM functionality is active                                           |
| SLEEP  | Reduced activity mode used when vehicle activity is not required             |
| FAULT  | BCM has detected a critical condition requiring restricted or safe operation |

---

## 4.3.2 BCM State Diagram

```text
                     Power On
                        |
                        v
                +---------------+
                |     INIT      |
                +---------------+
                        |
              Initialization OK
                        |
                        v
                +---------------+
        +------>|    ACTIVE     |<------+
        |       +---------------+       |
        |               |               |
        |               |               |
        |        Sleep Conditions       |
        |               |               |
        |               v               |
        |       +---------------+       |
        +-------|     SLEEP     |-------+
        Wake    +---------------+   Wake Event
        Event
                        |
                        |
                 Critical Fault
                        |
                        v
                +---------------+
                |     FAULT     |
                +---------------+
                        |
                 Fault Recovery
                        |
                        v
                +---------------+
                |    ACTIVE     |
                +---------------+
```

---

## 4.3.3 INIT State

The BCM enters `INIT` immediately after power-up or reset.

The following operations are performed:

* HAL initialization.
* GPIO initialization.
* Timer initialization.
* CAN initialization.
* FreeRTOS initialization.
* Diagnostic initialization.
* Fault Manager initialization.
* Application module initialization.
* Initial input sampling.
* Initial output state setup.

Outputs shall remain in safe default states during initialization.

Example safe defaults:

```text
Headlamps       = OFF
High Beam       = OFF
Left Indicator  = OFF
Right Indicator = OFF
Wipers          = OFF
Door Lock Cmd   = INACTIVE
```

Transition:

```text
INIT -> ACTIVE
```

when initialization completes successfully.

If a critical initialization failure occurs:

```text
INIT -> FAULT
```

---

## 4.3.4 ACTIVE State

`ACTIVE` is the normal vehicle operating state.

In this state:

* Inputs are processed.
* Lighting functions operate.
* Door locking functions operate.
* Wiper functions operate.
* CAN communication is active.
* Diagnostics are processed.
* Fault monitoring is active.

The BCM remains in `ACTIVE` while normal operating conditions are present.

---

## 4.3.5 SLEEP State

The `SLEEP` state represents reduced ECU activity.

Possible sleep-entry conditions may include:

* Ignition OFF.
* No active BCM requests.
* No diagnostic session active.
* No wake-capable CAN activity.
* No critical fault requiring active monitoring.

In the host-simulation version, sleep behavior may initially be simulated rather than implementing true MCU low-power mode.

Possible wake events include:

* Ignition ON.
* Door input change.
* Lock/unlock request.
* CAN wake event.
* Diagnostic request.

Transition:

```text
SLEEP -> ACTIVE
```

when a valid wake event occurs.

---

## 4.3.6 FAULT State

The BCM shall enter `FAULT` when a critical system condition prevents normal operation.

Examples include:

* Internal software failure.
* Critical communication failure.
* Invalid system state.
* Critical initialization failure.

The BCM shall place affected outputs into defined safe states.

Recovery may occur automatically or after reset depending on the fault.

---

# 4.4 Lighting State Machine

The Lighting State Machine controls the main exterior lighting operating mode.

Initial states:

```text
LIGHT_STATE_OFF
LIGHT_STATE_LOW_BEAM
LIGHT_STATE_HIGH_BEAM
```

## 4.4.1 Lighting State Diagram

```text
                  Light Switch ON
        +--------------------------------+
        |                                |
        v                                |
+---------------+                  +---------------+
|      OFF      |----------------->|   LOW_BEAM    |
+---------------+                  +---------------+
        ^                                |
        |                                |
        |                          High Beam ON
        |                                |
        |                                v
        |                         +---------------+
        +-------------------------|   HIGH_BEAM   |
          Light Switch OFF        +---------------+
                                        |
                                  High Beam OFF
                                        |
                                        v
                                  +---------------+
                                  |   LOW_BEAM    |
                                  +---------------+
```

---

## 4.4.2 OFF State

Outputs:

```text
Low Beam  = OFF
High Beam = OFF
```

Transition to `LOW_BEAM` when:

```text
Light switch = ON
```

---

## 4.4.3 LOW_BEAM State

Outputs:

```text
Low Beam  = ON
High Beam = OFF
```

Transition to `HIGH_BEAM` when:

```text
High beam request = ACTIVE
```

Transition to `OFF` when:

```text
Light switch = OFF
```

---

## 4.4.4 HIGH_BEAM State

Outputs:

```text
Low Beam  = ON
High Beam = ON
```

or another final output behavior may be selected depending on the simulated vehicle design.

Transition to `LOW_BEAM` when:

```text
High beam request = INACTIVE
```

Transition to `OFF` when:

```text
Main light switch = OFF
```

---

# 4.5 Turn Indicator State Machine

The turn indicator system shall support:

* Left indicator.
* Right indicator.
* Hazard warning.

Because flashing behavior depends on time, the indicator logic uses a timing-based state machine.

States:

```text
INDICATOR_STATE_OFF
INDICATOR_STATE_ON
```

The logical request determines which lamps participate in the flashing sequence.

---

## 4.5.1 Basic Flash State Diagram

```text
                 Flash Request
                      |
                      v
              +---------------+
              |      ON       |
              +---------------+
                      |
                 500 ms elapsed
                      |
                      v
              +---------------+
              |      OFF      |
              +---------------+
                      |
                 500 ms elapsed
                      |
                      +-----------> ON
```

If the request is removed:

```text
ON  -> OFF
OFF -> OFF
```

The indicator output shall immediately return to OFF.

---

## 4.5.2 Left Turn Request

When:

```text
Left Turn Request = ACTIVE
Hazard Request    = INACTIVE
```

the outputs shall flash:

```text
Left Indicator  = FLASHING
Right Indicator = OFF
```

---

## 4.5.3 Right Turn Request

When:

```text
Right Turn Request = ACTIVE
Hazard Request     = INACTIVE
```

the outputs shall flash:

```text
Left Indicator  = OFF
Right Indicator = FLASHING
```

---

# 4.6 Hazard Warning State Machine

Hazard warning has priority over normal left and right turn requests.

When:

```text
Hazard Request = ACTIVE
```

both indicators shall flash together.

```text
Left Indicator  = FLASHING
Right Indicator = FLASHING
```

State behavior:

```text
          Hazard ON
             |
             v
      +---------------+
      |  HAZARD_ON    |
      +---------------+
             |
        Flash Timer
             |
             v
       Toggle Outputs
             |
             +----------+
             |          |
             +----------+
```

When the hazard request becomes inactive, normal turn-signal processing resumes.

---

## 4.6.1 Indicator Request Priority

The initial priority shall be:

```text
Hazard Request
     |
     v
Left / Right Turn Request
     |
     v
Indicators OFF
```

Therefore:

```text
Hazard = ON
Left   = ON
```

results in:

```text
Both Indicators Flash
```

rather than left-only flashing.

---

# 4.7 Door Lock State Machine

The Door Lock State Machine controls the logical lock state of the vehicle.

States:

```text
DOOR_LOCK_STATE_LOCKED
DOOR_LOCK_STATE_UNLOCKED
```

---

## 4.7.1 Door Lock State Diagram

```text
                  Unlock Request
             +----------------------+
             |                      |
             v                      |
      +---------------+      +---------------+
      |    LOCKED     |      |   UNLOCKED    |
      +---------------+      +---------------+
             ^                      |
             |                      |
             +----------------------+
                   Lock Request
```

---

## 4.7.2 LOCKED State

The BCM considers the vehicle locked.

Logical output:

```text
Vehicle Lock State = LOCKED
```

Transition:

```text
LOCKED -> UNLOCKED
```

when a valid unlock request is received.

---

## 4.7.3 UNLOCKED State

The BCM considers the vehicle unlocked.

Transition:

```text
UNLOCKED -> LOCKED
```

when a valid lock request is received.

---

## 4.7.4 Door Lock Command Handling

Lock and unlock actuator commands shall be treated as requests rather than permanent output levels.

For example:

```text
Lock Request
     |
     v
Activate Lock Output
     |
     v
Command Duration Expires
     |
     v
Deactivate Output
```

This prevents the actuator command from remaining permanently energized.

Exact command timing shall be defined during implementation.

---

# 4.8 Wiper State Machine

The Wiper State Machine controls windshield wiper behavior.

States:

```text
WIPER_STATE_OFF
WIPER_STATE_INTERMITTENT
WIPER_STATE_LOW
WIPER_STATE_HIGH
```

---

## 4.8.1 Wiper State Diagram

```text
                     +----------------+
                     |      OFF       |
                     +----------------+
                       /      |      \
                      /       |       \
                     v        v        v
            +-------------+ +-----+ +------+
            |INTERMITTENT | | LOW | | HIGH |
            +-------------+ +-----+ +------+
                   ^          ^        ^
                   |          |        |
                   +----------+--------+
                      Wiper Switch
                         Change
```

The selected state is primarily determined by the wiper switch input.

---

## 4.8.2 OFF State

Output:

```text
Wiper Motor Request = OFF
```

Transitions are based on the wiper switch:

```text
OFF -> INTERMITTENT
OFF -> LOW
OFF -> HIGH
```

---

## 4.8.3 INTERMITTENT State

The wiper performs one wiping action at defined intervals.

Conceptual behavior:

```text
Wait Interval
     |
     v
Wiper Sweep
     |
     v
Return / Park
     |
     v
Wait Interval
```

No blocking delay shall be used.

Time shall be tracked using FreeRTOS timing or the Timer HAL.

---

## 4.8.4 LOW State

Output:

```text
Wiper Motor Speed = LOW
```

The wiper operates continuously at low speed.

---

## 4.8.5 HIGH State

Output:

```text
Wiper Motor Speed = HIGH
```

The wiper operates continuously at high speed.

---

# 4.9 Wiper Intermittent Sub-State Machine

The intermittent wiper mode may use an internal sub-state machine.

States:

```text
WIPER_INT_WAIT
WIPER_INT_SWEEP
```

Conceptual behavior:

```text
       +---------------+
       |     WAIT      |
       +---------------+
               |
        Interval elapsed
               |
               v
       +---------------+
       |     SWEEP     |
       +---------------+
               |
         Sweep complete
               |
               v
       +---------------+
       |     WAIT      |
       +---------------+
```

This allows the application to simulate realistic intermittent wiper behavior without blocking other BCM tasks.

---

# 4.10 Diagnostic Session State Machine

The Diagnostic Service shall maintain the current diagnostic session.

Initial states:

```text
DIAG_SESSION_DEFAULT
DIAG_SESSION_EXTENDED
```

Additional sessions may be added later if required.

---

## 4.10.1 Diagnostic State Diagram

```text
              Extended Session Request
        +------------------------------+
        |                              |
        v                              |
+------------------+          +------------------+
| DEFAULT SESSION  |          | EXTENDED SESSION |
+------------------+          +------------------+
        ^                              |
        |                              |
        +------------------------------+
         Default Session / Timeout
```

---

## 4.10.2 Default Session

The BCM starts in:

```text
DIAG_SESSION_DEFAULT
```

Only services allowed in the default session shall be processed.

---

## 4.10.3 Extended Session

The BCM enters the extended diagnostic session after receiving a valid request.

Additional diagnostic functionality may become available in this state.

A diagnostic inactivity timeout may cause:

```text
EXTENDED -> DEFAULT
```

---

# 4.11 CAN Communication State Machine

The CAN communication service may track communication health using the following states:

```text
CAN_STATE_INIT
CAN_STATE_ACTIVE
CAN_STATE_TIMEOUT
CAN_STATE_FAULT
```

---

## 4.11.1 CAN State Diagram

```text
          Initialization
                |
                v
        +---------------+
        |     INIT      |
        +---------------+
                |
           CAN Ready
                |
                v
        +---------------+
        |    ACTIVE     |
        +---------------+
                |
         Message Timeout
                |
                v
        +---------------+
        |    TIMEOUT    |
        +---------------+
           |         |
Message Rx |         | Repeated /
Recovered  |         | Critical Failure
           v         v
       ACTIVE      FAULT
```

---

## 4.11.2 CAN TIMEOUT State

If a periodically expected CAN message is not received within its timeout period:

* A communication timeout fault shall be reported.
* A DTC may be set.
* The affected application signal shall use a defined default or safe value.

When valid communication resumes:

```text
TIMEOUT -> ACTIVE
```

depending on the fault recovery strategy.

---

# 4.12 Fault State Handling

Each application or service module may detect local faults.

Fault reporting shall follow:

```text
Fault Condition
      |
      v
Detecting Module
      |
      v
FaultManager_SetFault()
      |
      v
Fault Stored
      |
      +-----------------------+
      |                       |
      v                       v
Application Safe State     Diagnostic DTC
```

Not every fault requires the entire BCM to enter `BCM_STATE_FAULT`.

Faults shall be classified by severity.

Example classification:

| Fault Severity | Example Response                    |
| -------------- | ----------------------------------- |
| Low            | Store fault and continue            |
| Medium         | Disable affected feature            |
| High           | Force affected output to safe state |
| Critical       | Enter BCM FAULT state               |

The exact severity assignments shall be defined during implementation.

---

# 4.13 Invalid State Handling

Each state machine shall include a default condition.

Example:

```c
switch (wiper_state)
{
    case WIPER_STATE_OFF:
        break;

    case WIPER_STATE_INTERMITTENT:
        break;

    case WIPER_STATE_LOW:
        break;

    case WIPER_STATE_HIGH:
        break;

    default:
        wiper_state = WIPER_STATE_OFF;
        FaultManager_SetFault(FAULT_INVALID_WIPER_STATE);
        break;
}
```

Unexpected states shall:

1. Be detected.
2. Be reported to the Fault Manager where appropriate.
3. Transition to a defined safe state.

---

# 4.14 Time-Based State Transitions

Several BCM features depend on time.

Examples include:

* Indicator flashing.
* Door lock actuator pulse.
* Intermittent wiper interval.
* Diagnostic session timeout.
* CAN communication timeout.
* Input debounce timing.

Blocking code such as:

```c
delay_ms(500);
```

shall not be used in normal application state-machine logic.

Instead, timing shall be implemented using either:

* FreeRTOS tick timing.
* FreeRTOS software timers.
* Timer HAL services.

Example:

```c
if ((current_time_ms - last_toggle_time_ms) >= 500U)
{
    last_toggle_time_ms = current_time_ms;
    indicator_output = !indicator_output;
}
```

An RTOS-based implementation may also use:

```c
vTaskDelayUntil();
```

for periodic FreeRTOS task execution.

The application logic itself should remain as independent as practical from direct RTOS calls so that it can still be unit tested outside FreeRTOS.

---

# 4.15 FreeRTOS Integration

State machines shall execute within appropriate FreeRTOS tasks.

A possible task structure is:

```text
+------------------------------------------------+
|                FreeRTOS Scheduler              |
+------------------------------------------------+
        |            |           |          |
        v            v           v          v
   Input Task   Body Task    CAN Task   Diagnostic Task
     10 ms        10 ms       10 ms        100 ms
                  |
                  v
          +----------------+
          | Lighting FSM   |
          | Wiper FSM      |
          | Door Lock FSM  |
          | BCM FSM        |
          +----------------+
```

One possible application design is to execute several lightweight state machines from one periodic Body Control task.

Example:

```c
void BodyControlTask(void *argument)
{
    TickType_t last_wake_time;

    last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        BcmManager_MainFunction();

        Lighting_MainFunction();

        Wiper_MainFunction();

        DoorLock_MainFunction();

        vTaskDelayUntil(&last_wake_time,
                        pdMS_TO_TICKS(10U));
    }
}
```

The exact FreeRTOS task structure may evolve during implementation.

The architecture shall avoid creating unnecessary tasks when several small state machines can execute safely within the same periodic task.

---

# 4.16 State Machine and Input Separation

Raw inputs shall not directly determine hardware outputs.

The preferred data flow is:

```text
Raw Input
   |
   v
HAL
   |
   v
Input Processing
   |
   v
Validated Logical Input
   |
   v
State Machine
   |
   v
Output Request
   |
   v
HAL
   |
   v
Hardware / Simulator
```

Example:

```text
Raw Wiper Switch
      |
      v
GPIO HAL
      |
      v
Input Processing
      |
      v
WIPER_SWITCH_LOW
      |
      v
Wiper State Machine
      |
      v
WIPER_STATE_LOW
      |
      v
Motor Output Request
```

---

# 4.17 State Machine and CAN Interaction

Some state transitions may be triggered by CAN signals.

Example:

```text
CAN Message
    |
    v
CAN HAL
    |
    v
CAN Service
    |
    v
Decoded Signal
    |
    v
BCM Application
    |
    v
State Transition
```

Example use cases include:

* Vehicle ignition state.
* Vehicle speed information.
* Remote lock/unlock request.
* External lighting request.
* Diagnostic commands.

Application state machines shall not decode raw CAN frames directly.

Raw CAN decoding shall remain within the CAN Service.

---

# 4.18 State Machine and Fault Manager Interaction

Application state machines shall report abnormal conditions to the Fault Manager.

Example:

```text
Wiper State Machine
       |
 Detect Invalid State
       |
       v
Fault Manager
       |
       +-------------------+
       |                   |
       v                   v
Store Fault         Diagnostic Service
       |
       v
Return Wiper to OFF
```

This keeps fault storage and diagnostic handling separate from individual application modules.

---

# 4.19 State Machine Unit Testing

Every major state machine shall be unit tested.

Tests shall verify:

* Correct initialization state.
* Valid state transitions.
* Invalid transition handling.
* Timing-based transitions.
* Output requests for each state.
* Fault response.
* Recovery behavior.

Example test:

```c
void test_wiper_should_transition_from_off_to_low(void)
{
    Wiper_Init();

    Wiper_SetRequestedMode(WIPER_MODE_LOW);

    Wiper_MainFunction();

    TEST_ASSERT_EQUAL(WIPER_STATE_LOW,
                      Wiper_GetState());
}
```

Indicator timing example:

```c
void test_left_indicator_should_toggle_after_flash_period(void)
{
    Lighting_Init();

    Lighting_SetLeftTurnRequest(true);

    TestTimer_SetTimeMs(0U);
    Lighting_MainFunction();

    TestTimer_SetTimeMs(500U);
    Lighting_MainFunction();

    TEST_ASSERT_TRUE(
        Lighting_GetLeftIndicatorState()
    );
}
```

Actual test APIs may differ during implementation.

---

# 4.20 Initial State Summary

The initial state machines planned for the BCM are summarized below.

| State Machine      | States                       |
| ------------------ | ---------------------------- |
| BCM Operating Mode | INIT, ACTIVE, SLEEP, FAULT   |
| Main Lighting      | OFF, LOW_BEAM, HIGH_BEAM     |
| Indicator Flasher  | OFF, ON                      |
| Door Lock          | LOCKED, UNLOCKED             |
| Wiper              | OFF, INTERMITTENT, LOW, HIGH |
| Intermittent Wiper | WAIT, SWEEP                  |
| Diagnostic Session | DEFAULT, EXTENDED            |
| CAN Communication  | INIT, ACTIVE, TIMEOUT, FAULT |

Additional states may be introduced where testing or implementation shows that more detailed behavior is required.

---

# 4.21 Software State Definitions

Suggested enumerations are shown below.

## BCM State

```c
typedef enum
{
    BCM_STATE_INIT = 0,
    BCM_STATE_ACTIVE,
    BCM_STATE_SLEEP,
    BCM_STATE_FAULT
} Bcm_State_t;
```

## Lighting State

```c
typedef enum
{
    LIGHT_STATE_OFF = 0,
    LIGHT_STATE_LOW_BEAM,
    LIGHT_STATE_HIGH_BEAM
} Lighting_State_t;
```

## Indicator State

```c
typedef enum
{
    INDICATOR_STATE_OFF = 0,
    INDICATOR_STATE_ON
} Indicator_State_t;
```

## Door Lock State

```c
typedef enum
{
    DOOR_LOCK_STATE_UNLOCKED = 0,
    DOOR_LOCK_STATE_LOCKED
} DoorLock_State_t;
```

## Wiper State

```c
typedef enum
{
    WIPER_STATE_OFF = 0,
    WIPER_STATE_INTERMITTENT,
    WIPER_STATE_LOW,
    WIPER_STATE_HIGH
} Wiper_State_t;
```

## Diagnostic State

```c
typedef enum
{
    DIAG_SESSION_DEFAULT = 0,
    DIAG_SESSION_EXTENDED
} DiagnosticSession_t;
```

## CAN State

```c
typedef enum
{
    CAN_STATE_INIT = 0,
    CAN_STATE_ACTIVE,
    CAN_STATE_TIMEOUT,
    CAN_STATE_FAULT
} Can_State_t;
```

---

# 4.22 Implementation Guideline

A typical application state-machine module should follow the structure:

```text
module.h
module.c
```

Example:

```text
wiper.h
wiper.c
```

The header file should expose only the required external interface.

Example:

```c
void Wiper_Init(void);

void Wiper_MainFunction(void);

void Wiper_SetRequestedMode(Wiper_Mode_t mode);

Wiper_State_t Wiper_GetState(void);
```

The internal state variable should normally remain private:

```c
static Wiper_State_t wiper_state;
```

Other modules shall not modify it directly.

---

# 4.23 Overall BCM State-Machine Interaction

The major state machines interact conceptually as follows:

```text
                   +----------------+
                   |    BCM FSM     |
                   +-------+--------+
                           |
                     BCM ACTIVE
                           |
          +----------------+----------------+
          |                |                |
          v                v                v
 +----------------+ +---------------+ +---------------+
 | Lighting FSM   | |   Wiper FSM   | | Door Lock FSM |
 +----------------+ +---------------+ +---------------+
          |                |                |
          +----------------+----------------+
                           |
                           v
                    Output Requests
                           |
                           v
                          HAL

          CAN Service ------+
                 |          |
                 v          |
           Application Inputs
                            |
          Diagnostics ------+
                 |
                 v
           Fault Manager
```

The BCM Manager controls overall ECU operating state, while individual feature state machines control their own functionality.

---

# 4.24 Summary

The BCM software uses explicit state machines to implement deterministic automotive control behavior.

The primary state machines are:

* BCM operating state.
* Lighting state.
* Indicator and hazard flashing.
* Door locking.
* Wiper control.
* Diagnostic sessions.
* CAN communication health.

The state machines shall:

* Be non-blocking.
* Use defined safe states.
* Integrate with the Fault Manager.
* Use processed inputs rather than raw hardware signals.
* Run periodically within FreeRTOS tasks.
* Remain independently testable where practical.
* Avoid direct hardware and raw CAN access.

The next design document is:

```text
docs/05_can_database.md
```

which defines the CAN message identifiers, signals, data layout, transmission periods, and communication behavior used by the BCM.
