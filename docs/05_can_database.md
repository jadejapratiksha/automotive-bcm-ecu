# 5. CAN Database

## 5.1 Purpose

This document defines the Controller Area Network (CAN) communication database for the Automotive Body Control Module (BCM) ECU project.

The CAN database specifies:

* CAN message identifiers.
* Message direction.
* Message transmission period.
* Data length.
* Signal names.
* Signal bit positions.
* Signal lengths.
* Signal scaling.
* Signal valid ranges.
* Signal default values.
* Timeout behavior.
* Application behavior when communication is lost.

The initial project uses Classical CAN with 11-bit standard identifiers and an 8-byte data payload.

The CAN design is intentionally simplified for educational and simulation purposes while following common automotive embedded-software concepts.

---

# 5.2 CAN Network Overview

The simulated vehicle network contains the BCM and several logical external ECUs.

Example network:

```text
                     +------------------+
                     |  Instrument ECU  |
                     +--------+---------+
                              |
                              |
+-------------+        +------+-------+        +---------------+
|  Door ECU   |--------|   CAN BUS    |--------| Engine / VCU  |
+-------------+        +------+-------+        +---------------+
                              |
                              |
                     +--------+---------+
                     |       BCM        |
                     +------------------+
```

For this project, the external ECUs may be represented by host-based simulation software rather than physical controllers.

The BCM shall receive vehicle-state information and remote requests through CAN and shall transmit BCM status information back onto the CAN network.

---

# 5.3 CAN Configuration

The initial CAN network parameters are:

| Parameter        | Value                                               |
| ---------------- | --------------------------------------------------- |
| CAN Type         | Classical CAN                                       |
| Identifier Type  | 11-bit Standard ID                                  |
| Payload Size     | Up to 8 bytes                                       |
| Nominal Bit Rate | 500 kbit/s                                          |
| Byte Order       | Intel / Little Endian                               |
| Error Detection  | CAN protocol CRC and application timeout monitoring |

The actual physical CAN bit rate is relevant when the firmware is later deployed on real hardware.

For host-based simulation, the communication interface shall preserve the same logical CAN identifiers and payload formats.

---

# 5.4 CAN Message Summary

The initial BCM CAN database contains the following messages.

| CAN ID | Message Name         | Direction Relative to BCM | DLC | Period |
| ------ | -------------------- | ------------------------- | --: | -----: |
| 0x100  | VEHICLE_STATUS       | RX                        |   8 |  20 ms |
| 0x110  | DRIVER_SWITCH_STATUS | RX                        |   8 |  20 ms |
| 0x120  | REMOTE_COMMAND       | RX                        |   8 |  Event |
| 0x200  | BCM_LIGHTING_STATUS  | TX                        |   8 | 100 ms |
| 0x210  | BCM_BODY_STATUS      | TX                        |   8 | 100 ms |
| 0x220  | BCM_FAULT_STATUS     | TX                        |   8 | 500 ms |
| 0x700  | DIAG_REQUEST         | RX                        |   8 |  Event |
| 0x708  | DIAG_RESPONSE        | TX                        |   8 |  Event |

RX means the BCM receives the message.

TX means the BCM transmits the message.

---

# 5.5 Message: VEHICLE_STATUS

## 5.5.1 General Information

```text
Message Name : VEHICLE_STATUS
CAN ID       : 0x100
Direction    : RX
DLC          : 8 bytes
Period       : 20 ms
Timeout      : 100 ms
```

This message provides general vehicle operating information to the BCM.

---

## 5.5.2 Signal Layout

| Byte | Bit(s) | Signal          | Length | Description                      |
| ---: | -----: | --------------- | -----: | -------------------------------- |
|    0 |      0 | Ignition_On     |  1 bit | Vehicle ignition status          |
|    0 |      1 | Engine_Running  |  1 bit | Engine/vehicle propulsion active |
|    0 |      2 | Vehicle_Moving  |  1 bit | Vehicle moving indication        |
|    0 |    3-7 | Reserved        | 5 bits | Reserved                         |
|    1 |    0-7 | Vehicle_Speed   | 8 bits | Vehicle speed                    |
|    2 |    0-7 | Battery_Voltage | 8 bits | Scaled supply voltage            |
|  3-7 |      - | Reserved        |      - | Reserved                         |

---

## 5.5.3 Signal Definitions

### Ignition_On

```text
Start Bit     : 0
Length        : 1 bit
Type          : Unsigned
Factor        : 1
Offset        : 0
```

Values:

| Raw Value | Meaning      |
| --------: | ------------ |
|         0 | Ignition OFF |
|         1 | Ignition ON  |

Default value:

```text
0
```

### Engine_Running

```text
Start Bit : 1
Length    : 1 bit
```

Values:

```text
0 = Engine / propulsion inactive
1 = Engine / propulsion active
```

### Vehicle_Moving

```text
Start Bit : 2
Length    : 1 bit
```

Values:

```text
0 = Vehicle stationary
1 = Vehicle moving
```

### Vehicle_Speed

```text
Start Bit : 8
Length    : 8 bits
Factor    : 1 km/h per bit
Offset    : 0
Range     : 0 to 250 km/h
```

Example:

```text
Raw = 65
Vehicle Speed = 65 km/h
```

### Battery_Voltage

```text
Start Bit : 16
Length    : 8 bits
Factor    : 0.1 V/bit
Offset    : 0
```

Example:

```text
Raw value = 125

Battery Voltage = 125 × 0.1
                = 12.5 V
```

---

## 5.5.4 Timeout Behavior

If `VEHICLE_STATUS` is not received for more than:

```text
100 ms
```

the BCM shall declare a communication timeout.

The BCM shall:

* Report a CAN timeout fault.
* Set the associated DTC.
* Set vehicle-state signals to safe default values where appropriate.
* Continue operating functions that do not depend on this message.

Example safe values:

```text
Ignition_On   = 0
Engine_Running = 0
Vehicle_Moving = 0
Vehicle_Speed = 0
```

---

# 5.6 Message: DRIVER_SWITCH_STATUS

## 5.6.1 General Information

```text
Message Name : DRIVER_SWITCH_STATUS
CAN ID       : 0x110
Direction    : RX
DLC          : 8
Period       : 20 ms
Timeout      : 100 ms
```

This message represents driver switch inputs that may originate from another ECU or simulated input panel.

---

## 5.6.2 Signal Layout

| Byte | Bit(s) | Signal             |
| ---: | -----: | ------------------ |
|    0 |      0 | Light_Switch       |
|    0 |      1 | High_Beam_Request  |
|    0 |      2 | Left_Turn_Request  |
|    0 |      3 | Right_Turn_Request |
|    0 |      4 | Hazard_Request     |
|    0 |      5 | Lock_Request       |
|    0 |      6 | Unlock_Request     |
|    0 |      7 | Reserved           |
|    1 |    0-1 | Wiper_Mode         |
|    1 |    2-7 | Reserved           |
|  2-7 |      - | Reserved           |

---

## 5.6.3 Light_Switch

```text
Start Bit : 0
Length    : 1
```

Values:

```text
0 = Exterior lighting OFF
1 = Low beam request
```

---

## 5.6.4 High_Beam_Request

```text
Start Bit : 1
Length    : 1
```

Values:

```text
0 = High beam inactive
1 = High beam requested
```

---

## 5.6.5 Left_Turn_Request

```text
Start Bit : 2
Length    : 1
```

Values:

```text
0 = Left turn request inactive
1 = Left turn request active
```

---

## 5.6.6 Right_Turn_Request

```text
Start Bit : 3
Length    : 1
```

Values:

```text
0 = Right turn request inactive
1 = Right turn request active
```

---

## 5.6.7 Hazard_Request

```text
Start Bit : 4
Length    : 1
```

Values:

```text
0 = Hazard switch OFF
1 = Hazard switch ON
```

Hazard request has priority over individual turn-indicator requests.

---

## 5.6.8 Lock_Request

```text
Start Bit : 5
Length    : 1
```

Values:

```text
0 = No lock request
1 = Lock request active
```

---

## 5.6.9 Unlock_Request

```text
Start Bit : 6
Length    : 1
```

Values:

```text
0 = No unlock request
1 = Unlock request active
```

---

## 5.6.10 Wiper_Mode

```text
Start Bit : 8
Length    : 2 bits
```

Values:

| Raw Value | Wiper Mode   |
| --------: | ------------ |
|         0 | OFF          |
|         1 | INTERMITTENT |
|         2 | LOW          |
|         3 | HIGH         |

---

## 5.6.11 Timeout Behavior

If the message times out:

```text
All switch requests shall default to inactive.
```

Example:

```text
Light_Switch       = 0
High_Beam_Request  = 0
Left_Turn_Request  = 0
Right_Turn_Request = 0
Hazard_Request     = 0
Lock_Request       = 0
Unlock_Request     = 0
Wiper_Mode         = OFF
```

The corresponding communication timeout fault shall be reported to the Fault Manager.

---

# 5.7 Message: REMOTE_COMMAND

## 5.7.1 General Information

```text
Message Name : REMOTE_COMMAND
CAN ID       : 0x120
Direction    : RX
DLC          : 8
Transmission : Event driven
```

This message represents remote vehicle commands such as commands from a key fob, gateway, or telematics ECU.

---

## 5.7.2 Signal Layout

| Byte | Bit(s) | Signal        |
| ---: | -----: | ------------- |
|    0 |      0 | Remote_Lock   |
|    0 |      1 | Remote_Unlock |
|    0 |      2 | Remote_Hazard |
|    0 |    3-7 | Reserved      |
|  1-7 |      - | Reserved      |

---

## 5.7.3 Remote_Lock

```text
0 = No request
1 = Remote lock request
```

---

## 5.7.4 Remote_Unlock

```text
0 = No request
1 = Remote unlock request
```

---

## 5.7.5 Remote_Hazard

```text
0 = No request
1 = Hazard flash request
```

This signal may later be used for lock/unlock confirmation flashing.

---

# 5.8 Message: BCM_LIGHTING_STATUS

## 5.8.1 General Information

```text
Message Name : BCM_LIGHTING_STATUS
CAN ID       : 0x200
Direction    : TX
DLC          : 8
Period       : 100 ms
```

This message reports the current BCM lighting output state.

---

## 5.8.2 Signal Layout

| Byte | Bit(s) | Signal                 |
| ---: | -----: | ---------------------- |
|    0 |      0 | Low_Beam_Status        |
|    0 |      1 | High_Beam_Status       |
|    0 |      2 | Left_Indicator_Status  |
|    0 |      3 | Right_Indicator_Status |
|    0 |      4 | Hazard_Status          |
|    0 |      5 | Interior_Lamp_Status   |
|    0 |    6-7 | Reserved               |
|  1-7 |      - | Reserved               |

---

## 5.8.3 Low_Beam_Status

```text
0 = Low beam OFF
1 = Low beam ON
```

---

## 5.8.4 High_Beam_Status

```text
0 = High beam OFF
1 = High beam ON
```

---

## 5.8.5 Left_Indicator_Status

```text
0 = Left indicator OFF
1 = Left indicator ON
```

This signal reports the instantaneous lamp output.

Therefore, during flashing the value alternates between:

```text
0
1
0
1
...
```

---

## 5.8.6 Right_Indicator_Status

```text
0 = Right indicator OFF
1 = Right indicator ON
```

---

## 5.8.7 Hazard_Status

```text
0 = Hazard function inactive
1 = Hazard function active
```

Unlike the individual indicator status signals, this represents the logical hazard operating state.

---

# 5.9 Message: BCM_BODY_STATUS

## 5.9.1 General Information

```text
Message Name : BCM_BODY_STATUS
CAN ID       : 0x210
Direction    : TX
DLC          : 8
Period       : 100 ms
```

This message reports body-control status information.

---

## 5.9.2 Signal Layout

| Byte | Bit(s) | Signal         |
| ---: | -----: | -------------- |
|    0 |      0 | Vehicle_Locked |
|    0 |    1-2 | Wiper_Status   |
|    0 |      3 | BCM_Active     |
|    0 |      4 | BCM_Sleep      |
|    0 |      5 | BCM_Fault      |
|    0 |    6-7 | Reserved       |
|  1-7 |      - | Reserved       |

---

## 5.9.3 Vehicle_Locked

```text
0 = Vehicle unlocked
1 = Vehicle locked
```

---

## 5.9.4 Wiper_Status

```text
Length : 2 bits
```

Values:

| Raw | Meaning      |
| --: | ------------ |
|   0 | OFF          |
|   1 | INTERMITTENT |
|   2 | LOW          |
|   3 | HIGH         |

---

## 5.9.5 BCM_Active

```text
0 = BCM not in ACTIVE state
1 = BCM ACTIVE
```

---

## 5.9.6 BCM_Sleep

```text
0 = BCM not in SLEEP state
1 = BCM SLEEP
```

---

## 5.9.7 BCM_Fault

```text
0 = No critical BCM fault
1 = Critical BCM fault active
```

---

# 5.10 Message: BCM_FAULT_STATUS

## 5.10.1 General Information

```text
Message Name : BCM_FAULT_STATUS
CAN ID       : 0x220
Direction    : TX
DLC          : 8
Period       : 500 ms
```

This message provides a simplified network-level fault status.

---

## 5.10.2 Signal Layout

| Byte | Signal                    |
| ---: | ------------------------- |
|    0 | Active Fault Count        |
|    1 | Communication Fault Flags |
|    2 | Application Fault Flags   |
|    3 | Hardware Fault Flags      |
|  4-7 | Reserved                  |

---

## 5.10.3 Active_Fault_Count

```text
Byte       : 0
Length     : 8 bits
Range      : 0 to 255
```

This field contains the number of currently active BCM faults.

---

## 5.10.4 Communication Fault Flags

Byte 1 contains communication-related faults.

Initial definition:

| Bit | Fault                        |
| --: | ---------------------------- |
|   0 | VEHICLE_STATUS timeout       |
|   1 | DRIVER_SWITCH_STATUS timeout |
|   2 | CAN controller fault         |
|   3 | Invalid CAN DLC              |
| 4-7 | Reserved                     |

---

## 5.10.5 Application Fault Flags

Byte 2 contains application-related faults.

Initial definition:

| Bit | Fault                   |
| --: | ----------------------- |
|   0 | Invalid lighting state  |
|   1 | Invalid wiper state     |
|   2 | Invalid door-lock state |
|   3 | Invalid BCM state       |
| 4-7 | Reserved                |

---

## 5.10.6 Hardware Fault Flags

Byte 3 contains hardware-related faults.

During host simulation these faults may be artificially injected.

Initial definition:

| Bit | Fault                |
| --: | -------------------- |
|   0 | GPIO output fault    |
|   1 | GPIO input fault     |
|   2 | Timer fault          |
|   3 | CAN peripheral fault |
| 4-7 | Reserved             |

---

# 5.11 Diagnostic CAN Messages

Diagnostics shall use dedicated CAN identifiers.

The initial addressing scheme is:

```text
Diagnostic Request  : 0x700
Diagnostic Response : 0x708
```

This is a simplified educational implementation.

A production automotive ECU may use ISO-TP and standardized diagnostic addressing.

---

# 5.12 Message: DIAG_REQUEST

## 5.12.1 General Information

```text
Message Name : DIAG_REQUEST
CAN ID       : 0x700
Direction    : RX
DLC          : 8
Transmission : Event driven
```

The payload shall contain the diagnostic request.

Example:

```text
Byte 0 = Service ID
Byte 1 = Parameter / Sub-function
Byte 2 = Parameter
...
```

---

## 5.12.2 Example Diagnostic Request

Read Data By Identifier:

```text
CAN ID: 0x700

Data:
22 F1 90 00 00 00 00 00
```

Conceptually:

```text
0x22 = Read Data By Identifier
0xF190 = Requested Data Identifier
```

Exact supported identifiers are defined in:

```text
docs/06_diagnostics.md
```

---

# 5.13 Message: DIAG_RESPONSE

## 5.13.1 General Information

```text
Message Name : DIAG_RESPONSE
CAN ID       : 0x708
Direction    : TX
DLC          : 8
Transmission : Event driven
```

The BCM shall use this message to return diagnostic responses.

---

## 5.13.2 Example Positive Response

Request:

```text
22 F1 90
```

Possible response:

```text
62 F1 90 xx xx xx xx xx
```

where:

```text
0x62
```

is the positive response service identifier corresponding to service `0x22`.

---

# 5.14 CAN Receive Processing

CAN receive processing shall remain separated from application logic.

Data flow:

```text
CAN Frame
   |
   v
CAN HAL
   |
   v
CAN Service
   |
   +----------------------+
   |                      |
Validate CAN ID        Validate DLC
   |                      |
   +----------+-----------+
              |
              v
        Decode Signals
              |
              v
       Application Data
              |
              v
       BCM State Machines
```

Application modules shall not directly decode raw CAN bytes.

---

# 5.15 CAN Transmit Processing

Transmit processing shall follow:

```text
Application State
      |
      v
CAN Service
      |
      v
Encode Signals
      |
      v
Construct CAN Frame
      |
      v
CAN HAL
      |
      v
CAN Bus / Simulator
```

For example:

```text
Lighting Module
      |
      v
Low Beam = ON
      |
      v
CAN Service
      |
      v
Set BCM_LIGHTING_STATUS Byte 0 Bit 0
      |
      v
Transmit ID 0x200
```

---

# 5.16 CAN Signal Decoding Example

Example received frame:

```text
CAN ID = 0x110

Data:
1D 02 00 00 00 00 00 00
```

Byte 0:

```text
0x1D = 0001 1101 binary
```

Therefore:

```text
Bit 0 Light_Switch        = 1
Bit 1 High_Beam_Request   = 0
Bit 2 Left_Turn_Request   = 1
Bit 3 Right_Turn_Request  = 1
Bit 4 Hazard_Request      = 1
Bit 5 Lock_Request        = 0
Bit 6 Unlock_Request      = 0
```

Byte 1:

```text
0x02
```

The lower two bits are:

```text
10 binary = 2
```

Therefore:

```text
Wiper_Mode = LOW
```

Because `Hazard_Request = 1`, the hazard function shall override the individual left/right indicator requests.

---

# 5.17 CAN Signal Encoding Example

Assume the BCM currently has:

```text
Low Beam       = ON
High Beam      = OFF
Left Indicator = ON
Right Indicator = OFF
Hazard         = OFF
Interior Lamp  = ON
```

For `BCM_LIGHTING_STATUS`:

```text
Bit 0 = 1
Bit 1 = 0
Bit 2 = 1
Bit 3 = 0
Bit 4 = 0
Bit 5 = 1
```

Byte 0 becomes:

```text
0010 0101
```

which is:

```text
0x25
```

Therefore the CAN frame may be:

```text
ID   : 0x200
DLC  : 8
Data : 25 00 00 00 00 00 00 00
```

---

# 5.18 Signal Storage

Decoded CAN signals shall be stored in structured application data rather than scattered global variables.

Example:

```c
typedef struct
{
    bool ignition_on;
    bool engine_running;
    bool vehicle_moving;

    uint8_t vehicle_speed_kph;

    uint16_t battery_voltage_mv;

} VehicleStatus_t;
```

Driver inputs may be stored as:

```c
typedef enum
{
    WIPER_MODE_OFF = 0,
    WIPER_MODE_INTERMITTENT,
    WIPER_MODE_LOW,
    WIPER_MODE_HIGH
} Wiper_Mode_t;

typedef struct
{
    bool light_switch;
    bool high_beam_request;

    bool left_turn_request;
    bool right_turn_request;

    bool hazard_request;

    bool lock_request;
    bool unlock_request;

    Wiper_Mode_t wiper_mode;

} DriverSwitchStatus_t;
```

The CAN Service shall update these structures when valid messages are received.

---

# 5.19 CAN Message Validation

Every received CAN message shall be validated before application data is updated.

Validation shall include:

* CAN identifier.
* DLC.
* Signal range where applicable.
* Message timeout monitoring.

Example:

```c
if (frame->dlc != 8U)
{
    FaultManager_SetFault(
        FAULT_CAN_INVALID_DLC
    );

    return;
}
```

Invalid messages shall not overwrite valid application state with uncontrolled data.

---

# 5.20 CAN Timeout Monitoring

Periodic CAN messages shall maintain a last-received timestamp.

Example:

```c
typedef struct
{
    uint32_t last_rx_time_ms;
    bool timeout_active;
} CanMessageMonitor_t;
```

When a message is received:

```c
monitor.last_rx_time_ms = TimerHal_GetTickMs();
monitor.timeout_active = false;
```

Periodic monitoring:

```c
if ((current_time_ms - monitor.last_rx_time_ms) >
    MESSAGE_TIMEOUT_MS)
{
    monitor.timeout_active = true;

    FaultManager_SetFault(
        FAULT_CAN_MESSAGE_TIMEOUT
    );
}
```

The production implementation may use FreeRTOS timing services or the Timer HAL depending on the final task structure.

---

# 5.21 CAN Recovery Behavior

When a timed-out message is received again and passes validation:

```text
TIMEOUT
   |
Valid Message Received
   |
   v
ACTIVE
```

The CAN Service shall:

* Update decoded signals.
* Mark communication as restored.
* Clear or mark the timeout fault as recovered according to the Fault Manager design.

The DTC may remain stored as historical even after the active fault is cleared.

---

# 5.22 Endianness

The initial CAN signal database uses:

```text
Intel / Little Endian
```

for multibyte values.

Example:

A 16-bit value:

```text
0x1234
```

shall be placed as:

```text
Byte 0 = 0x34
Byte 1 = 0x12
```

All encode and decode functions shall use the same defined byte order.

---

# 5.23 Reserved Bits

Unused bits shall be marked as reserved.

Reserved transmit bits shall normally be transmitted as:

```text
0
```

Reserved receive bits shall be ignored.

Application logic shall not rely on reserved fields.

---

# 5.24 CAN Service API

A possible CAN Service interface is:

```c
void CanService_Init(void);

void CanService_MainFunction(void);

void CanService_RxIndication(
    uint32_t can_id,
    const uint8_t *data,
    uint8_t length
);

const VehicleStatus_t *
CanService_GetVehicleStatus(void);

const DriverSwitchStatus_t *
CanService_GetDriverSwitchStatus(void);
```

Transmit functionality may include:

```c
void CanService_TransmitLightingStatus(void);

void CanService_TransmitBodyStatus(void);

void CanService_TransmitFaultStatus(void);
```

The final implementation may refine these APIs.

---

# 5.25 CAN HAL Interface

The CAN Service shall not directly interact with STM32 CAN registers.

Example HAL interface:

```c
typedef struct
{
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} Can_Frame_t;
```

Possible functions:

```c
void CanHal_Init(void);

bool CanHal_Transmit(
    const Can_Frame_t *frame
);

bool CanHal_Receive(
    Can_Frame_t *frame
);

void CanHal_MainFunction(void);
```

The host implementation may simulate CAN traffic using software queues or another local communication mechanism.

The STM32 implementation may later map the same interface to the STM32 FDCAN/CAN peripheral.

---

# 5.26 FreeRTOS Integration

CAN communication shall execute within the FreeRTOS architecture.

A possible structure is:

```text
             CAN Hardware / Simulator
                      |
                      v
                  CAN HAL
                      |
                      v
                +-----------+
                | CAN Task  |
                +-----------+
                  /       \
                 /         \
                v           v
            RX Decode    TX Scheduling
                |           |
                v           v
          Application     CAN HAL
             Data
```

Example task:

```c
void CanTask(void *argument)
{
    TickType_t last_wake_time;

    last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        CanHal_MainFunction();

        CanService_MainFunction();

        vTaskDelayUntil(
            &last_wake_time,
            pdMS_TO_TICKS(10U)
        );
    }
}
```

CAN reception may later use interrupt notification, queues, or task notifications when running on actual STM32 hardware.

---

# 5.27 CAN Reception Queue

When using FreeRTOS, received CAN frames may be passed from the low-level driver to the CAN task using a queue.

Conceptual architecture:

```text
CAN Interrupt
     |
     v
Read CAN Frame
     |
     v
FreeRTOS Queue
     |
     v
CAN Task
     |
     v
Decode Frame
     |
     v
Application Data
```

Example conceptual queue:

```c
QueueHandle_t can_rx_queue;
```

The actual ISR and FreeRTOS queue implementation will be added during target integration if required.

---

# 5.28 Host CAN Simulation

Because physical hardware is not required for the initial project, CAN traffic shall be simulated.

The simulation shall allow test software to inject frames.

Example:

```text
Simulation Tool
      |
      | ID 0x110
      | DATA 15 00 00 00 00 00 00 00
      v
Host CAN HAL
      |
      v
CAN Service
      |
      v
BCM Application
```

Possible simulation methods include:

* C-based test harness.
* Python script.
* Socket-based CAN simulation.
* Linux SocketCAN when available.
* Virtual CAN interface.
* Unit tests using mocked CAN frames.

The exact simulation transport may evolve without changing the defined CAN message database.

---

# 5.29 Python CAN Test Tool

A future Python test utility may generate CAN frames for the simulated BCM.

Example conceptual command:

```text
python send_can.py 0x110 15 00 00 00 00 00 00 00
```

The test script may later support human-readable commands such as:

```text
set light on
set left_turn on
set wiper low
lock vehicle
```

and convert them into the appropriate CAN frames.

This will provide a simple visual or console-based way to interact with the BCM simulation.

---

# 5.30 DBC Compatibility

The signal definitions in this document are structured so that they can later be converted into a standard CAN DBC file.

A future file may be added:

```text
dbc/bcm.dbc
```

The DBC file could be used with CAN tools such as:

* CANoe.
* CANalyzer.
* SavvyCAN.
* python-can.
* cantools.
* Other CAN simulation and analysis tools.

The Markdown database shall remain the human-readable design reference, while the DBC file may later become the machine-readable representation.

---

# 5.31 Initial CAN Faults

The following CAN-related faults are planned.

| Fault ID | Fault Name                   |
| -------- | ---------------------------- |
| 0x0101   | VEHICLE_STATUS_TIMEOUT       |
| 0x0102   | DRIVER_SWITCH_STATUS_TIMEOUT |
| 0x0103   | CAN_INVALID_DLC              |
| 0x0104   | CAN_CONTROLLER_FAULT         |
| 0x0105   | CAN_INVALID_MESSAGE          |
| 0x0106   | CAN_BUS_OFF                  |

Exact DTC numbering will be defined in:

```text
docs/06_diagnostics.md
```

---

# 5.32 CAN Message Matrix

The complete initial message matrix is summarized below.

| CAN ID | Name                 | BCM Direction | DLC | Type            | Timeout |
| ------ | -------------------- | ------------- | --: | --------------- | ------: |
| 0x100  | VEHICLE_STATUS       | RX            |   8 | 20 ms periodic  |  100 ms |
| 0x110  | DRIVER_SWITCH_STATUS | RX            |   8 | 20 ms periodic  |  100 ms |
| 0x120  | REMOTE_COMMAND       | RX            |   8 | Event           |     N/A |
| 0x200  | BCM_LIGHTING_STATUS  | TX            |   8 | 100 ms periodic |     N/A |
| 0x210  | BCM_BODY_STATUS      | TX            |   8 | 100 ms periodic |     N/A |
| 0x220  | BCM_FAULT_STATUS     | TX            |   8 | 500 ms periodic |     N/A |
| 0x700  | DIAG_REQUEST         | RX            |   8 | Event           |     N/A |
| 0x708  | DIAG_RESPONSE        | TX            |   8 | Event           |     N/A |

---

# 5.33 Detailed Signal Matrix

| Message              | Signal                 | Start Bit | Length | Factor | Unit |
| -------------------- | ---------------------- | --------: | -----: | -----: | ---- |
| VEHICLE_STATUS       | Ignition_On            |         0 |      1 |      1 | bool |
| VEHICLE_STATUS       | Engine_Running         |         1 |      1 |      1 | bool |
| VEHICLE_STATUS       | Vehicle_Moving         |         2 |      1 |      1 | bool |
| VEHICLE_STATUS       | Vehicle_Speed          |         8 |      8 |      1 | km/h |
| VEHICLE_STATUS       | Battery_Voltage        |        16 |      8 |    0.1 | V    |
| DRIVER_SWITCH_STATUS | Light_Switch           |         0 |      1 |      1 | bool |
| DRIVER_SWITCH_STATUS | High_Beam_Request      |         1 |      1 |      1 | bool |
| DRIVER_SWITCH_STATUS | Left_Turn_Request      |         2 |      1 |      1 | bool |
| DRIVER_SWITCH_STATUS | Right_Turn_Request     |         3 |      1 |      1 | bool |
| DRIVER_SWITCH_STATUS | Hazard_Request         |         4 |      1 |      1 | bool |
| DRIVER_SWITCH_STATUS | Lock_Request           |         5 |      1 |      1 | bool |
| DRIVER_SWITCH_STATUS | Unlock_Request         |         6 |      1 |      1 | bool |
| DRIVER_SWITCH_STATUS | Wiper_Mode             |         8 |      2 |      1 | enum |
| REMOTE_COMMAND       | Remote_Lock            |         0 |      1 |      1 | bool |
| REMOTE_COMMAND       | Remote_Unlock          |         1 |      1 |      1 | bool |
| REMOTE_COMMAND       | Remote_Hazard          |         2 |      1 |      1 | bool |
| BCM_LIGHTING_STATUS  | Low_Beam_Status        |         0 |      1 |      1 | bool |
| BCM_LIGHTING_STATUS  | High_Beam_Status       |         1 |      1 |      1 | bool |
| BCM_LIGHTING_STATUS  | Left_Indicator_Status  |         2 |      1 |      1 | bool |
| BCM_LIGHTING_STATUS  | Right_Indicator_Status |         3 |      1 |      1 | bool |
| BCM_LIGHTING_STATUS  | Hazard_Status          |         4 |      1 |      1 | bool |
| BCM_LIGHTING_STATUS  | Interior_Lamp_Status   |         5 |      1 |      1 | bool |
| BCM_BODY_STATUS      | Vehicle_Locked         |         0 |      1 |      1 | bool |
| BCM_BODY_STATUS      | Wiper_Status           |         1 |      2 |      1 | enum |
| BCM_BODY_STATUS      | BCM_Active             |         3 |      1 |      1 | bool |
| BCM_BODY_STATUS      | BCM_Sleep              |         4 |      1 |      1 | bool |
| BCM_BODY_STATUS      | BCM_Fault              |         5 |      1 |      1 | bool |

---

# 5.34 Design Rules

The following CAN design rules shall be followed:

* Application modules shall not directly parse raw CAN frames.
* CAN identifiers shall be defined in one central location.
* Encode and decode functions shall be separated from application behavior.
* All periodic receive messages shall have timeout monitoring.
* Invalid DLC values shall be rejected.
* Safe default values shall be defined for timed-out signals.
* Reserved transmit bits shall be zero.
* CAN communication faults shall be reported to the Fault Manager.
* CAN timing shall use non-blocking mechanisms.
* CAN processing shall integrate with FreeRTOS.
* CAN data structures shall use fixed-width integer types.
* CAN interfaces shall remain hardware-independent.
* Host simulation and STM32 execution shall use the same logical CAN database.

---

# 5.35 Summary

The Automotive BCM uses a defined CAN database to communicate vehicle inputs, driver requests, BCM status, faults, and diagnostic information.

The initial network uses the following primary CAN messages:

```text
0x100  VEHICLE_STATUS
0x110  DRIVER_SWITCH_STATUS
0x120  REMOTE_COMMAND

0x200  BCM_LIGHTING_STATUS
0x210  BCM_BODY_STATUS
0x220  BCM_FAULT_STATUS

0x700  DIAG_REQUEST
0x708  DIAG_RESPONSE
```

The CAN architecture provides:

* Periodic signal communication.
* Event-driven commands.
* Message timeout monitoring.
* Fault reporting.
* Diagnostics.
* Host-based CAN simulation.
* A future path to STM32 CAN/FDCAN hardware.
* Compatibility with a future DBC file.

The next design document is:

```text
docs/06_diagnostics.md
```

which defines Diagnostic Trouble Codes, supported diagnostic services, diagnostic sessions, positive and negative responses, and fault reporting behavior.
