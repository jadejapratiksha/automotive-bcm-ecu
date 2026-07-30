#include "unity.h"

#include "lighting_manager.h"

#include "mock_door_manager.h"
#include "mock_battery_monitor.h"
#include "mock_vehicle_state_manager.h"
#include "mock_gpio_driver.h"


void setUp(void)
{
}


void tearDown(void)
{
}


/*
 * Test 1:
 * Initialization must force the interior lamp OFF.
 */
void test_LightingManager_InitShouldTurnInteriorLampOff(void)
{
    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_LOW
    );

    LightingManager_Init();
}


/*
 * Test 2:
 * Door OPEN + normal battery + valid vehicle state
 * should turn the interior lamp ON.
 */
void test_LightingManager_ShouldTurnLampOnWhenDoorOpenAndConditionsValid(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_OPEN
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    BatteryMonitor_IsLowVoltage_ExpectAndReturn(
        false
    );

    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_HIGH
    );

    LightingManager_MainFunction();
}


/*
 * Test 3:
 * Door CLOSED should turn lamp OFF.
 */
void test_LightingManager_ShouldTurnLampOffWhenDoorClosed(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_CLOSED
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    BatteryMonitor_IsLowVoltage_ExpectAndReturn(
        false
    );

    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_LOW
    );

    LightingManager_MainFunction();
}


/*
 * Test 4:
 * Low battery condition must prevent lamp activation.
 */
void test_LightingManager_ShouldTurnLampOffWhenLowVoltageActive(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_OPEN
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    BatteryMonitor_IsLowVoltage_ExpectAndReturn(
        true
    );

    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_LOW
    );

    LightingManager_MainFunction();
}


/*
 * Test 5:
 * Invalid vehicle state should force lamp OFF.
 */
void test_LightingManager_ShouldTurnLampOffWhenVehicleStateInvalid(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_OPEN
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_MAX
    );

    BatteryMonitor_IsLowVoltage_ExpectAndReturn(
        false
    );

    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_LOW
    );

    LightingManager_MainFunction();
}


/*
 * Test 6:
 * Door OPEN while vehicle is in ACCESSORY state
 * should allow lamp activation.
 */
void test_LightingManager_ShouldTurnLampOnInAccessoryState(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_OPEN
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_ACCESSORY
    );

    BatteryMonitor_IsLowVoltage_ExpectAndReturn(
        false
    );

    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_HIGH
    );

    LightingManager_MainFunction();
}


/*
 * Test 7:
 * Door OPEN while vehicle is RUNNING
 * should allow lamp activation.
 */
void test_LightingManager_ShouldTurnLampOnInRunningState(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_OPEN
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_RUNNING
    );

    BatteryMonitor_IsLowVoltage_ExpectAndReturn(
        false
    );

    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_HIGH
    );

    LightingManager_MainFunction();
}


/*
 * Test 8:
 * Even when battery voltage is low, a CLOSED door
 * must still result in lamp OFF.
 */
void test_LightingManager_ShouldKeepLampOffWhenDoorClosedAndBatteryLow(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_CLOSED
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_RUNNING
    );

    BatteryMonitor_IsLowVoltage_ExpectAndReturn(
        true
    );

    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_LOW
    );

    LightingManager_MainFunction();
}


/*
 * Test 9:
 * Invalid vehicle state combined with low voltage
 * must keep lamp OFF.
 */
void test_LightingManager_ShouldKeepLampOffForMultipleInvalidConditions(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_OPEN
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_MAX
    );

    BatteryMonitor_IsLowVoltage_ExpectAndReturn(
        true
    );

    GPIO_Driver_Write_Expect(
        GPIO_CH_INTERIOR_LAMP,
        GPIO_STATE_LOW
    );

    LightingManager_MainFunction();
}