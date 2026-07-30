#include "unity.h"

#include "fault_manager.h"

#include "mock_battery_monitor.h"
#include "mock_vehicle_state_manager.h"


void setUp(void)
{
    FaultManager_Init();
}


void tearDown(void)
{
}


/*
 * Test 1:
 * No faults should be active after initialization.
 */
void test_FaultManager_ShouldStartWithNoActiveFaults(void)
{
    TEST_ASSERT_FALSE(
        FaultManager_IsAnyFaultActive()
    );

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        FaultManager_GetActiveFaultMask()
    );
}


/*
 * Test 2:
 * Setting BATTERY_LOW should activate that fault.
 */
void test_FaultManager_ShouldSetBatteryLowFault(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_LOW
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsAnyFaultActive()
    );
}


/*
 * Test 3:
 * Clearing BATTERY_LOW should remove the fault.
 */
void test_FaultManager_ShouldClearBatteryLowFault(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_LOW
    );

    FaultManager_ClearFault(
        FAULT_ID_BATTERY_LOW
    );

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );

    TEST_ASSERT_FALSE(
        FaultManager_IsAnyFaultActive()
    );
}


/*
 * Test 4:
 * Multiple faults should be active simultaneously.
 */
void test_FaultManager_ShouldSupportMultipleActiveFaults(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_LOW
    );

    FaultManager_SetFault(
        FAULT_ID_INVALID_VEHICLE_STATE
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_INVALID_VEHICLE_STATE
        )
    );
}


/*
 * Test 5:
 * Clearing one fault should not clear other faults.
 */
void test_FaultManager_ShouldClearOnlyRequestedFault(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_LOW
    );

    FaultManager_SetFault(
        FAULT_ID_BATTERY_CRITICAL
    );

    FaultManager_ClearFault(
        FAULT_ID_BATTERY_LOW
    );

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_CRITICAL
        )
    );
}


/*
 * Test 6:
 * Setting the same fault twice should remain safe.
 */
void test_FaultManager_SettingSameFaultTwiceShouldRemainActive(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_LOW
    );

    FaultManager_SetFault(
        FAULT_ID_BATTERY_LOW
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );
}


/*
 * Test 7:
 * Invalid fault ID should not modify fault mask.
 */
void test_FaultManager_ShouldIgnoreInvalidFaultIdWhenSetting(void)
{
    FaultManager_SetFault(
        FAULT_ID_MAX
    );

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        FaultManager_GetActiveFaultMask()
    );
}


/*
 * Test 8:
 * Clearing invalid ID should do nothing.
 */
void test_FaultManager_ShouldIgnoreInvalidFaultIdWhenClearing(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_LOW
    );

    FaultManager_ClearFault(
        FAULT_ID_MAX
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );
}


/*
 * Test 9:
 * Querying invalid ID should return false.
 */
void test_FaultManager_ShouldReturnFalseForInvalidFaultId(void)
{
    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_MAX
        )
    );
}


/*
 * Test 10:
 * Verify bit mask for BATTERY_LOW.
 *
 * FAULT_ID_BATTERY_LOW = 0
 * Therefore bit 0 should be set.
 */
void test_FaultManager_BatteryLowShouldSetBit0(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_LOW
    );

    TEST_ASSERT_EQUAL_UINT32(
        (1UL << 0),
        FaultManager_GetActiveFaultMask()
    );
}


/*
 * Test 11:
 * Verify bit mask for BATTERY_CRITICAL.
 *
 * FAULT_ID_BATTERY_CRITICAL = 1
 */
void test_FaultManager_BatteryCriticalShouldSetBit1(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_CRITICAL
    );

    TEST_ASSERT_EQUAL_UINT32(
        (1UL << 1),
        FaultManager_GetActiveFaultMask()
    );
}


/*
 * Test 12:
 * Verify bit mask for BATTERY_OVER_VOLTAGE.
 *
 * FAULT_ID_BATTERY_OVER_VOLTAGE = 2
 */
void test_FaultManager_BatteryOverVoltageShouldSetBit2(void)
{
    FaultManager_SetFault(
        FAULT_ID_BATTERY_OVER_VOLTAGE
    );

    TEST_ASSERT_EQUAL_UINT32(
        (1UL << 2),
        FaultManager_GetActiveFaultMask()
    );
}


/*
 * Test 13:
 * Verify bit mask for INVALID_VEHICLE_STATE.
 *
 * FAULT_ID_INVALID_VEHICLE_STATE = 3
 */
void test_FaultManager_InvalidVehicleStateShouldSetBit3(void)
{
    FaultManager_SetFault(
        FAULT_ID_INVALID_VEHICLE_STATE
    );

    TEST_ASSERT_EQUAL_UINT32(
        (1UL << 3),
        FaultManager_GetActiveFaultMask()
    );
}


/*
 * Test 14:
 * NORMAL battery + valid vehicle state
 * should produce no faults.
 */
void test_FaultManager_MainFunctionShouldReportNoFaultsForNormalConditions(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsAnyFaultActive()
    );
}


/*
 * Test 15:
 * LOW battery should activate BATTERY_LOW.
 */
void test_FaultManager_MainFunctionShouldSetBatteryLowFault(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_LOW
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_CRITICAL
        )
    );
}


/*
 * Test 16:
 * CRITICAL battery should activate
 * BATTERY_CRITICAL only.
 */
void test_FaultManager_MainFunctionShouldSetBatteryCriticalFault(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_CRITICAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_CRITICAL
        )
    );

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );
}


/*
 * Test 17:
 * OVER_VOLTAGE battery state should activate
 * over-voltage fault.
 */
void test_FaultManager_MainFunctionShouldSetBatteryOverVoltageFault(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_OVER_VOLTAGE
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_OVER_VOLTAGE
        )
    );
}


/*
 * Test 18:
 * Invalid vehicle state should set the
 * INVALID_VEHICLE_STATE fault.
 */
void test_FaultManager_MainFunctionShouldSetInvalidVehicleStateFault(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_MAX
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_INVALID_VEHICLE_STATE
        )
    );
}


/*
 * Test 19:
 * Valid vehicle states must not set invalid-state fault.
 */
void test_FaultManager_MainFunctionShouldNotSetInvalidFaultForRunningState(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_RUNNING
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_INVALID_VEHICLE_STATE
        )
    );
}


/*
 * Test 20:
 * Battery fault should automatically clear after
 * battery returns to NORMAL.
 */
void test_FaultManager_MainFunctionShouldClearBatteryFaultAfterRecovery(void)
{
    /*
     * First cycle:
     * Battery is LOW.
     */
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_LOW
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );


    /*
     * Second cycle:
     * Battery has recovered.
     */
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );

    TEST_ASSERT_FALSE(
        FaultManager_IsAnyFaultActive()
    );
}


/*
 * Test 21:
 * Transitioning from LOW to CRITICAL should clear
 * LOW and set CRITICAL.
 */
void test_FaultManager_ShouldTransitionFromLowFaultToCriticalFault(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_LOW
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );


    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_CRITICAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_CRITICAL
        )
    );
}


/*
 * Test 22:
 * Transitioning from CRITICAL to NORMAL should
 * clear the CRITICAL fault.
 */
void test_FaultManager_ShouldClearCriticalFaultAfterRecovery(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_CRITICAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();


    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_CRITICAL
        )
    );
}


/*
 * Test 23:
 * Over-voltage fault should clear after returning
 * to NORMAL battery state.
 */
void test_FaultManager_ShouldClearOverVoltageFaultAfterRecovery(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_OVER_VOLTAGE
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_OVER_VOLTAGE
        )
    );


    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_OVER_VOLTAGE
        )
    );
}


/*
 * Test 24:
 * Invalid vehicle-state fault should clear once
 * vehicle state becomes valid again.
 */
void test_FaultManager_ShouldClearInvalidVehicleStateFaultAfterRecovery(void)
{
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_MAX
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_INVALID_VEHICLE_STATE
        )
    );


    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_RUNNING
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_INVALID_VEHICLE_STATE
        )
    );
}


/*
 * Test 25:
 * Verify complete battery-fault progression:
 *
 * NORMAL
 *   ↓
 * LOW fault
 *   ↓
 * CRITICAL fault
 *   ↓
 * NORMAL / no battery fault
 */
void test_FaultManager_ShouldHandleCompleteBatteryFaultCycle(void)
{
    /*
     * NORMAL
     */
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsAnyFaultActive()
    );


    /*
     * LOW
     */
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_LOW
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );


    /*
     * CRITICAL
     */
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_CRITICAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_LOW
        )
    );

    TEST_ASSERT_TRUE(
        FaultManager_IsFaultActive(
            FAULT_ID_BATTERY_CRITICAL
        )
    );


    /*
     * NORMAL
     */
    BatteryMonitor_GetState_ExpectAndReturn(
        BATTERY_STATE_NORMAL
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    FaultManager_MainFunction();

    TEST_ASSERT_FALSE(
        FaultManager_IsAnyFaultActive()
    );
}