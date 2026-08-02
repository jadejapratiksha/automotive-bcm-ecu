"""
Python reference model for the Automotive BCM ECU.

This model mirrors the behavior currently implemented in the C firmware.
It does not communicate with the STM32 firmware yet. It provides an
independent expected-behavior model for later scenario and Renode tests.
"""

from dataclasses import dataclass
from enum import IntEnum


class VehicleState(IntEnum):
    """Equivalent to vehicle_state_t in the C firmware."""

    OFF = 0
    ACCESSORY = 1
    RUNNING = 2


class DoorState(IntEnum):
    """Equivalent to door_state_t in the C firmware."""

    CLOSED = 0
    OPEN = 1


class BatteryState(IntEnum):
    """Equivalent to battery_state_t in the C firmware."""

    NORMAL = 0
    LOW = 1
    CRITICAL = 2
    OVER_VOLTAGE = 3


class FaultId(IntEnum):
    """Equivalent to fault_id_t in the C firmware."""

    BATTERY_LOW = 0
    BATTERY_CRITICAL = 1
    BATTERY_OVER_VOLTAGE = 2
    INVALID_VEHICLE_STATE = 3


class DiagnosticCode(IntEnum):
    """Equivalent to diagnostic_code_t in the C firmware."""

    BATTERY_LOW = 0x1001
    BATTERY_CRITICAL = 0x1002
    BATTERY_OVER_VOLTAGE = 0x1003
    INVALID_VEHICLE_STATE = 0x1004


# Battery thresholds copied from battery_monitor.c.
BATTERY_CRITICAL_ENTER_MV = 9000
BATTERY_CRITICAL_EXIT_MV = 9500

BATTERY_LOW_ENTER_MV = 11000
BATTERY_LOW_EXIT_MV = 11500

BATTERY_OVER_VOLTAGE_ENTER_MV = 15500
BATTERY_OVER_VOLTAGE_EXIT_MV = 15000

BATTERY_DEFAULT_VOLTAGE_MV = 12000


@dataclass
class BCMOutputs:
    """
    Observable BCM outputs.

    These values can later be compared with output received from Renode.
    """

    interior_lamp_on: bool = False
    fault_mask: int = 0
    diagnostic_mask: int = 0


class BCMModel:
    """
    Reference behavioral model for the current BCM firmware.
    """

    def __init__(self) -> None:
        self.vehicle_state = VehicleState.OFF
        self.front_left_door = DoorState.CLOSED

        self.battery_voltage_mv = BATTERY_DEFAULT_VOLTAGE_MV
        self.battery_state = BatteryState.NORMAL

        self.outputs = BCMOutputs()

    def reset(self) -> None:
        """
        Return the model to the same startup conditions as the C firmware.
        """

        self.vehicle_state = VehicleState.OFF
        self.front_left_door = DoorState.CLOSED

        self.battery_voltage_mv = BATTERY_DEFAULT_VOLTAGE_MV
        self.battery_state = BatteryState.NORMAL

        self.outputs = BCMOutputs()

    def set_vehicle_state(self, requested_state: VehicleState) -> bool:
        """
        Request a vehicle-state transition.

        This mirrors VehicleStateManager_SetState().

        Valid transitions:

        OFF -> OFF or ACCESSORY
        ACCESSORY -> OFF, ACCESSORY or RUNNING
        RUNNING -> OFF, ACCESSORY or RUNNING

        Direct OFF -> RUNNING is rejected.
        """

        if not isinstance(requested_state, VehicleState):
            return False

        if self.vehicle_state == VehicleState.OFF:
            transition_valid = requested_state in (
                VehicleState.OFF,
                VehicleState.ACCESSORY,
            )

        elif self.vehicle_state == VehicleState.ACCESSORY:
            transition_valid = requested_state in (
                VehicleState.OFF,
                VehicleState.ACCESSORY,
                VehicleState.RUNNING,
            )

        elif self.vehicle_state == VehicleState.RUNNING:
            transition_valid = requested_state in (
                VehicleState.OFF,
                VehicleState.ACCESSORY,
                VehicleState.RUNNING,
            )

        else:
            transition_valid = False

        if transition_valid:
            self.vehicle_state = requested_state

        return transition_valid

    def set_front_left_door(self, door_state: DoorState) -> None:
        """
        Set the logical front-left door state.
        """

        if not isinstance(door_state, DoorState):
            raise ValueError(f"Invalid door state: {door_state}")

        self.front_left_door = door_state

    def set_battery_voltage_mv(self, voltage_mv: int) -> None:
        """
        Supply a simulated battery voltage in millivolts.

        Example:
            12000 means 12.0 volts.
        """

        if not isinstance(voltage_mv, int):
            raise TypeError("Battery voltage must be an integer.")

        if not 0 <= voltage_mv <= 65535:
            raise ValueError(
                "Battery voltage must fit within an unsigned 16-bit value."
            )

        self.battery_voltage_mv = voltage_mv

    def _update_battery_state(self) -> None:
        """
        Mirror BatteryMonitor_MainFunction() hysteresis behavior.
        """

        if self.battery_state == BatteryState.NORMAL:
            if self.battery_voltage_mv < BATTERY_CRITICAL_ENTER_MV:
                self.battery_state = BatteryState.CRITICAL

            elif self.battery_voltage_mv < BATTERY_LOW_ENTER_MV:
                self.battery_state = BatteryState.LOW

            elif self.battery_voltage_mv > BATTERY_OVER_VOLTAGE_ENTER_MV:
                self.battery_state = BatteryState.OVER_VOLTAGE

        elif self.battery_state == BatteryState.LOW:
            if self.battery_voltage_mv < BATTERY_CRITICAL_ENTER_MV:
                self.battery_state = BatteryState.CRITICAL

            elif self.battery_voltage_mv >= BATTERY_LOW_EXIT_MV:
                self.battery_state = BatteryState.NORMAL

        elif self.battery_state == BatteryState.CRITICAL:
            if self.battery_voltage_mv >= BATTERY_CRITICAL_EXIT_MV:
                if self.battery_voltage_mv < BATTERY_LOW_EXIT_MV:
                    self.battery_state = BatteryState.LOW
                else:
                    self.battery_state = BatteryState.NORMAL

        elif self.battery_state == BatteryState.OVER_VOLTAGE:
            if self.battery_voltage_mv <= BATTERY_OVER_VOLTAGE_EXIT_MV:
                self.battery_state = BatteryState.NORMAL

        else:
            self.battery_state = BatteryState.NORMAL

    def _update_interior_lamp(self) -> None:
        """
        Mirror LightingManager_MainFunction().

        The interior lamp turns on when:

        - The front-left door is open.
        - Battery state is not LOW or CRITICAL.
        - Vehicle state is valid.

        Over-voltage does not currently disable the lamp in the C firmware.
        """

        low_voltage_active = self.battery_state in (
            BatteryState.LOW,
            BatteryState.CRITICAL,
        )

        vehicle_state_valid = isinstance(
            self.vehicle_state,
            VehicleState,
        )

        self.outputs.interior_lamp_on = (
            self.front_left_door == DoorState.OPEN
            and not low_voltage_active
            and vehicle_state_valid
        )

    def _update_faults(self) -> None:
        """
        Mirror FaultManager_MainFunction().
        """

        fault_mask = 0

        if self.battery_state == BatteryState.LOW:
            fault_mask |= 1 << FaultId.BATTERY_LOW

        if self.battery_state == BatteryState.CRITICAL:
            fault_mask |= 1 << FaultId.BATTERY_CRITICAL

        if self.battery_state == BatteryState.OVER_VOLTAGE:
            fault_mask |= 1 << FaultId.BATTERY_OVER_VOLTAGE

        if not isinstance(self.vehicle_state, VehicleState):
            fault_mask |= 1 << FaultId.INVALID_VEHICLE_STATE

        self.outputs.fault_mask = fault_mask

    def _update_diagnostics(self) -> None:
        """
        Mirror DiagnosticManager_MainFunction().

        In the current firmware, each diagnostic bit corresponds directly
        to the equivalent active fault bit.
        """

        diagnostic_mask = 0

        if self.is_fault_active(FaultId.BATTERY_LOW):
            diagnostic_mask |= 1 << 0

        if self.is_fault_active(FaultId.BATTERY_CRITICAL):
            diagnostic_mask |= 1 << 1

        if self.is_fault_active(FaultId.BATTERY_OVER_VOLTAGE):
            diagnostic_mask |= 1 << 2

        if self.is_fault_active(FaultId.INVALID_VEHICLE_STATE):
            diagnostic_mask |= 1 << 3

        self.outputs.diagnostic_mask = diagnostic_mask

    def tick(self) -> None:
        """
        Execute one simulated BCM processing cycle.

        Processing order follows the intended firmware dependency order:

        1. Battery monitoring
        2. Lighting control
        3. Fault management
        4. Diagnostic management
        """

        self._update_battery_state()
        self._update_interior_lamp()
        self._update_faults()
        self._update_diagnostics()

    def is_low_voltage(self) -> bool:
        """
        Equivalent to BatteryMonitor_IsLowVoltage().
        """

        return self.battery_state in (
            BatteryState.LOW,
            BatteryState.CRITICAL,
        )

    def is_vehicle_running(self) -> bool:
        """
        Equivalent to VehicleStateManager_IsRunning().
        """

        return self.vehicle_state == VehicleState.RUNNING

    def is_fault_active(self, fault_id: FaultId) -> bool:
        """
        Check whether a particular fault bit is active.
        """

        return bool(
            self.outputs.fault_mask
            & (1 << int(fault_id))
        )

    def is_dtc_active(self, dtc: DiagnosticCode) -> bool:
        """
        Check whether a diagnostic trouble code is active.
        """

        dtc_to_bit = {
            DiagnosticCode.BATTERY_LOW: 0,
            DiagnosticCode.BATTERY_CRITICAL: 1,
            DiagnosticCode.BATTERY_OVER_VOLTAGE: 2,
            DiagnosticCode.INVALID_VEHICLE_STATE: 3,
        }

        bit = dtc_to_bit.get(dtc)

        if bit is None:
            return False

        return bool(
            self.outputs.diagnostic_mask
            & (1 << bit)
        )

    def get_status(self) -> dict:
        """
        Return the complete observable BCM state.
        """

        return {
            "vehicle_state": self.vehicle_state.name,
            "front_left_door": self.front_left_door.name,
            "battery_voltage_mv": self.battery_voltage_mv,
            "battery_state": self.battery_state.name,
            "interior_lamp_on": self.outputs.interior_lamp_on,
            "fault_mask": self.outputs.fault_mask,
            "diagnostic_mask": self.outputs.diagnostic_mask,
        }


def main() -> None:
    """
    Run a basic demonstration when this file is executed directly.
    """

    bcm = BCMModel()

    print("Initial BCM state:")
    print(bcm.get_status())

    print("\nOpening the front-left door...")
    bcm.set_front_left_door(DoorState.OPEN)
    bcm.tick()
    print(bcm.get_status())

    print("\nReducing the battery voltage to 10.5 V...")
    bcm.set_battery_voltage_mv(10500)
    bcm.tick()
    print(bcm.get_status())

    print("\nRestoring the battery voltage to 12.0 V...")
    bcm.set_battery_voltage_mv(12000)
    bcm.tick()
    print(bcm.get_status())


if __name__ == "__main__":
    main()