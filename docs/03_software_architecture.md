# 3. Software Architecture

## 3.1 Purpose

This document defines the software architecture for the Automotive Body Control Module (BCM) ECU project.

The software architecture is designed to:

* Separate application logic from hardware-specific code.
* Allow development and testing without a physical STM32 board.
* Support future integration with STM32 hardware.
* Provide modular and maintainable embedded C code.
* Support CAN communication and diagnostics.
* Allow individual software modules to be unit tested.
* Provide a structure suitable for automotive embedded firmware development.
* Use FreeRTOS for task scheduling and real-time software execution.


The initial implementation will be developed and tested using host-based simulation. Hardware-dependent functionality will be isolated behind abstraction interfaces so that simulated drivers can later be replaced with STM32 drivers without significantly changing the application software.

---

## 3.2 High-Level Software Architecture

The BCM software is divided into the following major layers:

1. Application Layer
2. Service Layer
3. Hardware Abstraction Layer (HAL)
4. Hardware / Simulation Layer

The general architecture is:

```text
+--------------------------------------------------+
|                 Application Layer                |
|--------------------------------------------------|
| Lighting | Door Lock | Wiper | BCM State Manager |
+--------------------------------------------------+
                       |
                       v
+--------------------------------------------------+
|                   Service Layer                  |
|--------------------------------------------------|
| CAN Service | Diagnostics | Fault Manager        |
| Scheduler   | Input Processing                   |
+--------------------------------------------------+
                       |
                       v
+--------------------------------------------------+
|          Hardware Abstraction Layer (HAL)        |
|--------------------------------------------------|
| GPIO HAL | CAN HAL | Timer HAL | ADC HAL         |
+--------------------------------------------------+
                       |
                       v
+--------------------------------------------------+
|             Hardware / Simulation Layer          |
|--------------------------------------------------|
| Host Simulation Drivers / Future STM32 Drivers   |
+--------------------------------------------------+
```

The application layer must not directly access STM32 registers or hardware peripherals.

All hardware access shall occur through the HAL interfaces.

---

## 3.3 Application Layer

The Application Layer contains the main functional behavior of the BCM.

The initial BCM application contains the following major modules:

```text
src/app/
|
+-- lighting/
|
+-- door_lock/
|
+-- wiper/
|
+-- bcm_manager/
```

Each module shall contain its own state, input processing, control logic, and output requests where appropriate.

### 3.3.1 Lighting Control Module

The Lighting Control Module controls exterior and interior lighting functions.

Responsibilities include:

* Low-beam headlamp control.
* High-beam headlamp control.
* Left turn indicator control.
* Right turn indicator control.
* Hazard warning operation.
* Interior/courtesy lamp control.
* Turn-signal flashing timing.
* Output state management.

Example interface:

```c
void Lighting_Init(void);
void Lighting_MainFunction(void);
```

The module receives processed input information and determines the required lighting outputs.

Hardware GPIO operations shall not be performed directly inside the lighting application logic.

### 3.3.2 Door Lock Control Module

The Door Lock Control Module manages vehicle locking and unlocking behavior.

Responsibilities include:

* Lock request processing.
* Unlock request processing.
* Door-state monitoring.
* Lock actuator command generation.
* Maintaining the current lock state.

Example interface:

```c
void DoorLock_Init(void);
void DoorLock_MainFunction(void);
```

### 3.3.3 Wiper Control Module

The Wiper Control Module manages windshield wiper operation.

Responsibilities include:

* Wiper OFF state.
* Intermittent wiping.
* Low-speed wiping.
* High-speed wiping.
* Wiper state transitions.
* Wiper output requests.

Example interface:

```c
void Wiper_Init(void);
void Wiper_MainFunction(void);
```

### 3.3.4 BCM Manager

The BCM Manager coordinates the overall BCM application.

Responsibilities include:

* Initialization of application modules.
* Vehicle/BCM operating-state management.
* Coordination between modules where required.
* Distribution of system-level information.
* Safe-state handling when required.

Example interface:

```c
void BcmManager_Init(void);
void BcmManager_MainFunction(void);
```

---

## 3.4 Service Layer

The Service Layer provides common services used by multiple application modules.

The major services are:

```text
src/services/
|
+-- can/
|
+-- diagnostics/
|
+-- fault_manager/
|
+-- scheduler/
|
+-- input_processing/
```

These services shall remain independent from specific application features whenever practical.

### 3.4.1 CAN Service

The CAN Service provides communication between the BCM and other simulated or future vehicle ECUs.

Responsibilities include:

* CAN message transmission.
* CAN message reception.
* Signal encoding.
* Signal decoding.
* Message timeout monitoring.
* Communication status tracking.

Example interface:

```c
void CanService_Init(void);
void CanService_MainFunction(void);
void CanService_RxIndication(uint32_t can_id,
                             const uint8_t *data,
                             uint8_t length);
```

The CAN Service shall use the CAN HAL rather than directly accessing CAN hardware.

### 3.4.2 Diagnostic Service

The Diagnostic Service provides basic ECU diagnostic functionality.

The initial implementation may support selected UDS-style diagnostic services such as:

* Diagnostic Session Control.
* ECU Reset.
* Read Data By Identifier.
* Read Diagnostic Trouble Codes.
* Clear Diagnostic Trouble Codes.

Example interface:

```c
void Diagnostics_Init(void);
void Diagnostics_MainFunction(void);
```

The diagnostic implementation is intentionally simplified for this educational project but shall follow automotive diagnostic concepts.

### 3.4.3 Fault Manager

The Fault Manager provides centralized fault handling.

Responsibilities include:

* Fault detection reporting.
* Fault status storage.
* Diagnostic Trouble Code management.
* Fault clearing.
* Fault status queries.

Example interface:

```c
void FaultManager_Init(void);
void FaultManager_SetFault(uint16_t fault_id);
void FaultManager_ClearFault(uint16_t fault_id);
bool FaultManager_IsFaultActive(uint16_t fault_id);
```

Application modules shall report detected faults to the Fault Manager rather than independently implementing DTC storage.

### 3.4.4 Scheduler

The Scheduler provides periodic execution of software tasks.

### 3.4.4 FreeRTOS Task Scheduling

The BCM software will use FreeRTOS to provide real-time task scheduling.

Major BCM functions will execute as separate periodic or event-driven tasks.

Example task periods may include:

| Task              | Period |
| ----------------- | -----: |
| Input processing  |  10 ms |
| Lighting control  |  10 ms |
| Door lock control |  20 ms |
| Wiper control     |  10 ms |
| CAN processing    |  10 ms |
| Diagnostics       | 100 ms |
| Fault monitoring  | 100 ms |

FreeRTOS services may be used for:

* Task scheduling.
* Software delays.
* Queues.
* Semaphores.
* Mutexes.
* Event groups.
* Inter-task communication.

Application logic shall remain separate from FreeRTOS-specific implementation wherever practical so that individual modules can still be unit tested on the host PC.

For example, a lighting control function may be called from a FreeRTOS task on the target system while the same function can be called directly from a unit test on the development PC.

The architecture shall avoid unnecessary blocking and shall ensure that high-priority tasks are not delayed by lower-priority processing.


Example task periods may include:

| Task              | Period |
| ----------------- | -----: |
| Input processing  |  10 ms |
| Lighting control  |  10 ms |
| Door lock control |  20 ms |
| Wiper control     |  10 ms |
| CAN processing    |  10 ms |
| Diagnostics       | 100 ms |
| Fault monitoring  | 100 ms |

Exact timing may be adjusted during implementation and testing.

The scheduler architecture is intentionally simple so that the firmware can execute both:

* On a PC using host simulation.
* On an STM32 microcontroller in the future.

FreeRTOS will be used as the real-time operating system for the BCM. Application and service functions will execute within periodic or event-driven FreeRTOS tasks. During early host-based development, selected modules may still be tested independently without the RTOS to simplify unit testing.

### 3.4.5 Input Processing

The Input Processing service converts raw hardware or simulated inputs into stable logical signals.

Responsibilities may include:

* Switch reading.
* Input validation.
* Debouncing.
* Signal normalization.
* Detection of input changes.

Application modules should consume processed logical inputs rather than directly reading GPIO pins.

---

## 3.5 Hardware Abstraction Layer

The Hardware Abstraction Layer isolates application and service software from physical hardware.

The HAL provides interfaces for:

* GPIO
* CAN
* Timers
* ADC
* System time

Example directory:

```text
src/hal/
|
+-- gpio_hal.c
+-- gpio_hal.h
+-- can_hal.c
+-- can_hal.h
+-- timer_hal.c
+-- timer_hal.h
+-- adc_hal.c
+-- adc_hal.h
```

Application code shall depend on these interfaces rather than directly depending on STM32 libraries.

### 3.5.1 GPIO HAL

The GPIO HAL provides digital input and output access.

Example interface:

```c
typedef enum
{
    GPIO_LOW = 0,
    GPIO_HIGH
} Gpio_State_t;

void Gpio_Init(void);
Gpio_State_t Gpio_Read(uint8_t channel);
void Gpio_Write(uint8_t channel, Gpio_State_t state);
```

### 3.5.2 CAN HAL

The CAN HAL provides the hardware-independent interface used by the CAN Service.

Example interface:

```c
void CanHal_Init(void);

bool CanHal_Transmit(uint32_t can_id,
                     const uint8_t *data,
                     uint8_t length);

void CanHal_MainFunction(void);
```

For host-based development, this interface may be connected to a software CAN simulator.

For future STM32 integration, the implementation can be replaced by an STM32 FDCAN/CAN driver.

### 3.5.3 Timer HAL

The Timer HAL provides timing information to the scheduler and application services.

Example interface:

```c
void TimerHal_Init(void);
uint32_t TimerHal_GetTickMs(void);
```

The host implementation may use operating-system timing functions.

The STM32 implementation may use a hardware timer or system tick.

### 3.5.4 ADC HAL

The ADC HAL provides access to analog input channels if analog sensors are added to the project.

Example interface:

```c
void AdcHal_Init(void);
uint16_t AdcHal_Read(uint8_t channel);
```

---

## 3.6 Hardware and Simulation Layer

The lowest software layer contains the platform-specific implementation.

Two platform implementations are planned.

### 3.6.1 Host Simulation Platform

The host simulation platform allows firmware development without physical hardware.

The simulation environment may provide:

* Simulated GPIO inputs.
* Simulated GPIO outputs.
* Simulated CAN messages.
* Simulated timers.
* Fault injection.
* Console output.
* Automated test execution.

Example:

```text
Input:
Left Turn Switch = ON

        |
        v

Lighting Control Logic

        |
        v

Simulated Output:
Left Indicator = FLASHING
```

This architecture allows application functionality to be verified before physical hardware is available.

### 3.6.2 STM32 Platform

A future STM32 platform implementation may use STM32Cube HAL or low-level drivers.

Example mapping:

```text
Generic BCM HAL
      |
      v
STM32 HAL Adapter
      |
      v
STM32 Peripheral
```

For example:

```text
Gpio_Write()
      |
      v
HAL_GPIO_WritePin()
      |
      v
Physical GPIO Pin
```

This design allows the same application software to run on both the host simulator and STM32 target.

---

## 3.7 Software Initialization Sequence

The BCM shall initialize software components in a controlled sequence.

Example initialization:

```text
System Start
     |
     v
HAL Initialization
     |
     v
Timer Initialization
     |
     v
GPIO Initialization
     |
     v
CAN Initialization
     |
     v
Fault Manager Initialization
     |
     v
Diagnostic Initialization
     |
     v
Application Module Initialization
     |
     v
Scheduler Start
     |
     v
Normal Operation
```

Example implementation:

```c
int main(void)
{
    Hal_Init();

    TimerHal_Init();
    Gpio_Init();
    CanHal_Init();

    FaultManager_Init();
    CanService_Init();
    Diagnostics_Init();

    Lighting_Init();
    DoorLock_Init();
    Wiper_Init();
    BcmManager_Init();

    Scheduler_Init();

    while (1)
    {
        Scheduler_Run();
    }

    return 0;
}
```

---

## 3.8 Cyclic Execution Model

The initial BCM software uses a cooperative cyclic execution model.

Conceptually:

```text
              +--------------------+
              |     Scheduler      |
              +---------+----------+
                        |
       +----------------+----------------+
       |                |                |
       v                v                v
     10 ms            20 ms            100 ms
       |                |                |
       v                v                v
   Lighting         Door Lock       Diagnostics
   Wiper                            Fault Manager
   CAN
```

Each task must execute quickly and return control to the scheduler.

Application modules shall avoid:

* Infinite loops.
* Long blocking delays.
* Busy waiting.
* Uncontrolled blocking operations.

For example, the following approach shall be avoided:

```c
while (switch_active)
{
    delay_ms(500);
    toggle_indicator();
}
```

Instead, timing shall be state-based:

```c
if ((current_time - previous_time) >= 500U)
{
    previous_time = current_time;
    indicator_state = !indicator_state;
}
```

This allows other BCM functions to continue operating while the indicator is flashing.

---

## 3.9 Module Communication

Modules shall communicate through defined interfaces rather than accessing each other's internal variables.

For example:

```text
Input Processing
       |
       v
Lighting Module
       |
       v
GPIO HAL
```

For CAN-controlled functions:

```text
CAN HAL
   |
   v
CAN Service
   |
   v
Decoded Vehicle Signals
   |
   v
Application Module
```

For fault reporting:

```text
Application Module
       |
       v
Fault Manager
       |
       v
Diagnostic Service
       |
       v
CAN Service
```

Global variables should be minimized.

Where shared data is required, access should occur through controlled APIs.

---

## 3.10 State Machine Architecture

State machines shall be used for functions whose behavior depends on previous state or operating mode.

Potential state-machine-controlled functions include:

* BCM operating mode.
* Wiper operation.
* Door locking.
* Turn signal operation.
* Diagnostic sessions.

Example:

```text
          Ignition ON
             |
             v
+-------+   ----->   +--------+
| SLEEP |            | ACTIVE |
+-------+   <-----    +--------+
             |
          Timeout /
         Ignition OFF
```

Detailed state machines are defined in:

```text
docs/04_state_machines.md
```

---

## 3.11 Fault Handling Architecture

Fault handling shall be centralized through the Fault Manager.

A typical fault path is:

```text
Fault Detected
      |
      v
Application / Service Module
      |
      v
Fault Manager
      |
      +------------------+
      |                  |
      v                  v
Store Fault          Safe Action
      |
      v
Diagnostic Trouble Code
      |
      v
Diagnostic Service
```

Examples of possible faults include:

* CAN communication timeout.
* Invalid received CAN data.
* Output command failure.
* Invalid switch state.
* Internal software state error.

Critical faults may cause affected outputs to transition to a predefined safe state.

---

## 3.12 CAN Software Data Flow

CAN communication shall follow the following receive path:

```text
CAN Frame
   |
   v
CAN HAL
   |
   v
CAN Service
   |
   v
Message Decoder
   |
   v
Application Signal
   |
   v
Application Module
```

Transmission follows the reverse direction:

```text
Application Module
   |
   v
Application Signal
   |
   v
CAN Service
   |
   v
Message Encoder
   |
   v
CAN HAL
   |
   v
CAN Frame
```

CAN message IDs and signal definitions are documented separately in:

```text
docs/05_can_database.md
```

---

## 3.13 Diagnostic Architecture

Diagnostic communication follows the general flow:

```text
Diagnostic Request
       |
       v
CAN HAL
       |
       v
CAN Service
       |
       v
Diagnostic Service
       |
       +-----------------------+
       |           |           |
       v           v           v
Read Data     Read DTCs    Clear DTCs
       |           |           |
       +-----------+-----------+
                   |
                   v
          Diagnostic Response
                   |
                   v
              CAN Service
                   |
                   v
                CAN HAL
```

Detailed diagnostic services and DTC definitions are documented in:

```text
docs/06_diagnostics.md
```

---

## 3.14 Testability

Testability is a major architectural requirement.

Application modules shall be designed so they can be compiled and tested on the development PC.

For example, the lighting module can be tested using:

```text
Simulated Input
      |
      v
Lighting Module
      |
      v
Mock GPIO HAL
      |
      v
Expected Output Verification
```

The project may use:

* Ceedling
* Unity
* CMock

for C unit testing.

Hardware-dependent functions shall be mocked during unit testing.

Example test concept:

```c
void test_left_turn_signal_should_activate_when_switch_is_on(void)
{
    Input_SetLeftTurnSwitch(true);

    Lighting_MainFunction();

    TEST_ASSERT_TRUE(Lighting_IsLeftIndicatorRequested());
}
```

This allows application logic to be tested without an STM32 board.

---

## 3.15 Coding and Design Principles

The software shall follow the following general principles:

* Modules shall have clearly defined responsibilities.
* Hardware access shall be isolated from application logic.
* Global variables shall be minimized.
* Interfaces shall be defined using header files.
* Application modules shall not directly access MCU registers.
* Blocking delays shall be avoided.
* State machines shall be used where appropriate.
* Fixed-width integer types shall be used where practical.
* Module initialization shall be explicit.
* Periodic functions shall have predictable execution behavior.
* Fault handling shall use centralized mechanisms.
* Code shall be designed for unit testing.
* Platform-dependent code shall remain isolated from platform-independent code.

Example fixed-width types:

```c
#include <stdint.h>
#include <stdbool.h>

uint8_t signal_state;
uint16_t fault_id;
uint32_t system_time_ms;
bool door_open;
```

---

## 3.16 Proposed Source Code Structure

The software architecture maps approximately to the following project structure:

```text
src/
|
+-- app/
|   |
|   +-- lighting/
|   |   +-- lighting.c
|   |   +-- lighting.h
|   |
|   +-- door_lock/
|   |   +-- door_lock.c
|   |   +-- door_lock.h
|   |
|   +-- wiper/
|   |   +-- wiper.c
|   |   +-- wiper.h
|   |
|   +-- bcm_manager/
|       +-- bcm_manager.c
|       +-- bcm_manager.h
|
+-- services/
|   |
|   +-- can/
|   |
|   +-- diagnostics/
|   |
|   +-- fault_manager/
|   |
|   +-- scheduler/
|   |
|   +-- input_processing/
|
+-- hal/
|   |
|   +-- gpio_hal.c
|   +-- gpio_hal.h
|   +-- can_hal.c
|   +-- can_hal.h
|   +-- timer_hal.c
|   +-- timer_hal.h
|
+-- platform/
|   |
|   +-- host/
|   |
|   +-- stm32/
|
+-- main.c
```

The exact directory structure may evolve during implementation, but the architectural separation between application, services, HAL, and platform-specific software shall be maintained.

---

## 3.17 Architecture Summary

The BCM software uses a modular layered architecture:

```text
+=============================================+
|              BCM APPLICATION                |
|---------------------------------------------|
| Lighting | Door Locks | Wipers | BCM Manager|
+=============================================+
                    |
                    v
+=============================================+
|                 SERVICES                    |
|---------------------------------------------|
| CAN | Diagnostics | Faults | Scheduler      |
+=============================================+
                    |
                    v
+=============================================+
|                    HAL                      |
|---------------------------------------------|
| GPIO | CAN | Timer | ADC                    |
+=============================================+
                    |
                    v
+=============================================+
|                  PLATFORM                   |
|---------------------------------------------|
|      Host Simulation  /  STM32 Target       |
+=============================================+
```

The key architectural objective is:

> The BCM application logic shall remain independent of the physical hardware platform.

This allows the project to begin entirely with host-based development and simulation while preserving a clear path to future STM32 hardware deployment.

The next design document is:

```text
docs/04_state_machines.md
```

which defines the detailed operating states and transitions for the BCM and its individual functions.
