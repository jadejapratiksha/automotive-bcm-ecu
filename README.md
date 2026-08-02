🚗 Automotive Body Control Module (BCM) ECU

Production-Style Automotive Firmware Project

STM32F407 • Embedded C • FreeRTOS • CAN • Renode • Ceedling • Unity • CMock • Python



📖 Table of Contents

Overview

Features

Technology Stack

System Architecture

FreeRTOS Task Design

CAN Protocol

Repository Structure

Unit Testing

Engineering Challenges

Lessons Learned

Future Roadmap

Documentation

Overview

This project implements a production-inspired Automotive Body Control Module (BCM) using an STM32F407 microcontroller and FreeRTOS. It demonstrates modern embedded firmware practices including layered architecture, CAN communication, diagnostics, hardware abstraction, automated unit testing, Renode simulation, GitHub Actions CI, and a Python GUI for firmware validation.

Features

Feature

Status

Layered Architecture

✅

FreeRTOS Multitasking

✅

CAN Communication

✅

Vehicle State Management

✅

Battery Monitoring

✅

Fault Management

✅

Diagnostic Trouble Codes

✅

Event Logging

✅

Python GUI

✅

Renode Simulation

✅

153 Automated Unit Tests

✅

GitHub Actions CI

✅

Technology Stack

Category

Technology

MCU

STM32F407

Language

Embedded C

RTOS

FreeRTOS

Communication

Classical CAN

Testing

Ceedling, Unity, CMock

Simulation

Renode

GUI

Python (Tkinter)

CI

GitHub Actions

System Architecture

                Python GUI
                     │
          UART / Virtual CAN
                     │
+------------------------------------------------+
|              BCM Firmware                      |
|------------------------------------------------|
| Vehicle State Manager                          |
| Door Manager                                   |
| Lighting Manager                               |
| Battery Monitor                                |
|------------------------------------------------|
| CAN Service                                    |
| Fault Manager                                  |
| Diagnostic Manager                             |
| Event Logger                                   |
|------------------------------------------------|
| GPIO Driver | ADC Driver | CAN Driver          |
+------------------------------------------------+
                     │
                STM32 HAL
                     │
            STM32F407 / Renode

FreeRTOS Task Design

Task

Period

Responsibility

VehicleTask

50 ms

Vehicle state & simulation commands

LightingTask

20 ms

Interior lighting

BatteryTask

100 ms

Battery monitoring

FaultTask

100 ms

Fault processing

DiagnosticTask

100 ms

DTC management

CANTask

10 ms

CAN RX/TX

CAN Protocol

Receive

CAN ID

Purpose

0x100

Ignition / Vehicle State Command

Transmit

CAN ID

Purpose

0x200

BCM Status

Repository Structure

firmware/
platform/
tests/
simulation/
tools/
docs/
.github/
README.md

Unit Testing

153 Tests Executed
153 Tests Passed
0 Failed
0 Ignored

Frameworks:

Ceedling

Unity

CMock

Engineering Challenges

Challenge 1 — CAN Initialization in Renode

Problem

The firmware entered Error_Handler() during CAN startup in Renode.

Debugging

Added UART checkpoints

Used addr2line

Inspected CAN registers

Compared simulator behavior with expected hardware behavior

Solution

Updated Renode CAN configuration

Started CAN from the dedicated CAN task

Verified stable execution

Challenge 2 — FreeRTOS Stack Size

Problem

The VehicleTask required more stack during integrated execution.

Solution

Increased VehicleTask stack allocation

Verified stable scheduling and communication

Lessons Learned

Separate application logic from hardware.

Unit-test firmware using mocked drivers.

Validate RTOS stack usage during integration.

Simulators require configuration that differs from physical hardware.

Build firmware incrementally and verify each subsystem.

Future Roadmap

Exterior lighting

Turn indicators

Hazard control

Door lock control

Wiper state machine

Extended CAN database

UDS diagnostics

Hardware-in-the-loop testing

Documentation

See the docs/ folder for:

Project Overview

Requirements

Software Architecture

State Machines

CAN Database

Diagnostics

Test Plan

License

This project is intended for educational, learning, and portfolio purposes.