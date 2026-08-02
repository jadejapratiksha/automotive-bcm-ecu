5. CAN Database

5.1 Purpose

This document defines the implemented CAN interface of the completed BCM firmware and separately records planned future messages.

The as-built protocol contains two Classical CAN messages.

5.2 Implemented CAN Configuration

Parameter

Value

CAN type

Classical CAN

Identifier format

11-bit standard

Maximum payload

8 bytes

Target bit rate

500 kbit/s

Byte order for multi-byte values

Little endian

The logical frame format is shared by the target firmware and simulation.

5.3 Implemented Message Summary

CAN ID

Name

Direction relative to BCM

DLC

Transmission

0x100

IGNITION_COMMAND

RX

1

Event-driven

0x200

BCM_STATUS

TX

8

Approximately 100 ms

5.4 Message 0x100 — IGNITION_COMMAND

General Information

CAN ID: 0x100
Direction: RX
DLC: 1

Payload

Byte

Signal

Type

Values

0

Ignition/vehicle-state command

uint8_t

0=OFF, 1=ACCESSORY, 2=RUNNING

Validation

The CAN Service shall verify:

CAN ID equals 0x100

DLC equals 1

Byte 0 contains a valid vehicle-state command

Invalid values shall not cause an uncontrolled state transition.

Example Frames

OFF:
ID 0x100, DLC 1, Data 00

ACCESSORY:
ID 0x100, DLC 1, Data 01

RUNNING:
ID 0x100, DLC 1, Data 02

5.5 Message 0x200 — BCM_STATUS

General Information

CAN ID: 0x200
Direction: TX
DLC: 8
Period: approximately 100 ms

The status frame is transmitted after a fixed number of CAN task iterations. With a 10 ms CAN task period and a 10-cycle status counter, the intended period is approximately 100 ms.

Payload Layout

Byte(s)

Signal

Description

0

Vehicle state

OFF / ACCESSORY / RUNNING

1

Front-left door state

CLOSED / OPEN

2

Battery voltage LSB

Millivolt value, little-endian

3

Battery voltage MSB

Millivolt value, little-endian

4

Active fault mask byte 0

Bits 0–7

5

Active fault mask byte 1

Bits 8–15

6

Active fault mask byte 2

Bits 16–23

7

Active fault mask byte 3

Bits 24–31

Battery Voltage Encoding

battery_voltage_mv = Byte2 + (Byte3 << 8)

Example:

Battery voltage = 12000 mV = 0x2EE0

Byte 2 = 0xE0
Byte 3 = 0x2E

Fault Mask Bits

Bit

Fault

0

Battery low

1

Battery critical

2

Battery over-voltage

3

Invalid vehicle state

4–31

Reserved

Reserved bits shall be zero unless future faults are added.

Example

Assume:

Vehicle state = RUNNING (2)

Door = OPEN (1)

Battery = 12000 mV (0x2EE0)

Battery-low fault active (0x00000001)

Frame:

ID: 0x200
DLC: 8
Data: 02 01 E0 2E 01 00 00 00

5.6 Receive Data Flow

CAN Frame
   |
   v
CAN Driver
   |
   v
CAN Service
   |
Validate ID / DLC / Command
   |
   v
VehicleStateManager_SetState()

Application modules do not decode raw CAN bytes directly.

5.7 Transmit Data Flow

Vehicle State
Door State
Battery Voltage
Fault Mask
   |
   v
CAN Service Encoder
   |
   v
CAN Driver
   |
   v
CAN Bus / Renode

5.8 FreeRTOS Integration

The CAN task performs:

CAN initialization/startup.

Receive polling.

Frame validation and processing.

Status-transmission scheduling.

Periodic delay.

Conceptual task:

for (;;)
{
    CANService_MainFunction();

    if (status_period_elapsed)
    {
        CANService_SendBCMStatus();
    }

    osDelay(10U);
}

Exact code may differ, but the logical behavior is the same.

5.9 Renode Considerations

A physical CAN network includes:

MCU CAN controller

CAN transceiver

Bus wiring

Termination

At least one connected network environment

Renode requires the corresponding virtual-bus configuration.

The initial simulation became stuck during HAL_CAN_Start() because the virtual CAN conditions were incomplete.

The final project includes Renode scripts for normal simulation and SocketCAN-oriented integration.

This simulator-specific setup does not change the application-level message definitions.

5.10 Error Handling

The current CAN implementation shall safely handle:

Unsupported CAN ID

Invalid DLC

Invalid state command

Receive function reporting no frame

The current project does not yet implement:

Message timeout DTCs

Bus-off recovery

Error-passive handling

Automatic retransmission strategy documentation

Network management

5.11 Planned Extended CAN Database

The following messages belong to the original design target and are not implemented in the completed firmware.

CAN ID

Planned name

Planned direction

0x110

DRIVER_SWITCH_STATUS

RX

0x120

REMOTE_COMMAND

RX

0x210

BCM_BODY_STATUS

TX

0x220

BCM_FAULT_STATUS

TX

0x700

DIAG_REQUEST

RX

0x708

DIAG_RESPONSE

TX

Potential planned signals include:

Headlight request/status

High beam

Indicators

Hazards

Door lock/unlock

Wiper mode

Communication timeout flags

Diagnostic services

These messages should be implemented and tested before being described as active protocol support.

5.12 Future DBC Support

A future machine-readable DBC may be generated from the implemented protocol and extended as new messages are added.

Recommended future file:

dbc/bcm.dbc

Possible tools:

cantools

python-can

SavvyCAN

CANoe

CANalyzer