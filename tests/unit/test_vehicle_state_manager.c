#include "unity.h"
#include "vehicle_state_manager.h"


void setUp(void)
{
    /*
     * Start every test from a known safe state.
     */
    VehicleStateManager_Init();
}


void tearDown(void)
{
}


/*
 * Test 1:
 * Initialization must place the vehicle in OFF state.
 */
void test_VehicleStateManager_ShouldStartInOffState(void)
{
    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 2:
 * Vehicle should not report RUNNING after initialization.
 */
void test_VehicleStateManager_ShouldNotBeRunningAfterInitialization(void)
{
    TEST_ASSERT_FALSE(
        VehicleStateManager_IsRunning()
    );
}


/*
 * Test 3:
 * OFF -> OFF is allowed.
 */
void test_VehicleStateManager_ShouldAllowOffToOffTransition(void)
{
    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_OFF)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 4:
 * OFF -> ACCESSORY is allowed.
 */
void test_VehicleStateManager_ShouldAllowOffToAccessoryTransition(void)
{
    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_ACCESSORY)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_ACCESSORY,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 5:
 * OFF -> RUNNING is NOT allowed directly.
 *
 * Based on your current state machine:
 *
 * OFF -> ACCESSORY -> RUNNING
 */
void test_VehicleStateManager_ShouldRejectOffToRunningTransition(void)
{
    TEST_ASSERT_FALSE(
        VehicleStateManager_SetState(VEHICLE_STATE_RUNNING)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 6:
 * ACCESSORY -> RUNNING is allowed.
 */
void test_VehicleStateManager_ShouldAllowAccessoryToRunningTransition(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_RUNNING)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_RUNNING,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 7:
 * ACCESSORY -> OFF is allowed.
 */
void test_VehicleStateManager_ShouldAllowAccessoryToOffTransition(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_OFF)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 8:
 * ACCESSORY -> ACCESSORY is allowed.
 */
void test_VehicleStateManager_ShouldAllowAccessoryToAccessoryTransition(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_ACCESSORY)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_ACCESSORY,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 9:
 * RUNNING -> RUNNING is allowed.
 */
void test_VehicleStateManager_ShouldAllowRunningToRunningTransition(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    VehicleStateManager_SetState(
        VEHICLE_STATE_RUNNING
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_RUNNING)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_RUNNING,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 10:
 * RUNNING -> ACCESSORY is allowed.
 */
void test_VehicleStateManager_ShouldAllowRunningToAccessoryTransition(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    VehicleStateManager_SetState(
        VEHICLE_STATE_RUNNING
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_ACCESSORY)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_ACCESSORY,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 11:
 * RUNNING -> OFF is allowed.
 */
void test_VehicleStateManager_ShouldAllowRunningToOffTransition(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    VehicleStateManager_SetState(
        VEHICLE_STATE_RUNNING
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_OFF)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 12:
 * IsRunning() must return TRUE in RUNNING state.
 */
void test_VehicleStateManager_IsRunningShouldReturnTrueWhenRunning(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    VehicleStateManager_SetState(
        VEHICLE_STATE_RUNNING
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_IsRunning()
    );
}


/*
 * Test 13:
 * IsRunning() must return FALSE in ACCESSORY state.
 */
void test_VehicleStateManager_IsRunningShouldReturnFalseWhenAccessory(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    TEST_ASSERT_FALSE(
        VehicleStateManager_IsRunning()
    );
}


/*
 * Test 14:
 * Invalid state VEHICLE_STATE_MAX must be rejected.
 */
void test_VehicleStateManager_ShouldRejectVehicleStateMax(void)
{
    TEST_ASSERT_FALSE(
        VehicleStateManager_SetState(VEHICLE_STATE_MAX)
    );

    /*
     * State must remain unchanged.
     */
    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 15:
 * A value above VEHICLE_STATE_MAX must also be rejected.
 */
void test_VehicleStateManager_ShouldRejectStateAboveMax(void)
{
    vehicle_state_t invalid_state =
        (vehicle_state_t)(VEHICLE_STATE_MAX + 1);

    TEST_ASSERT_FALSE(
        VehicleStateManager_SetState(invalid_state)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 16:
 * Rejecting an invalid transition must not modify
 * the existing state.
 *
 * OFF -> RUNNING is invalid.
 */
void test_VehicleStateManager_InvalidTransitionShouldPreserveCurrentState(void)
{
    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );

    TEST_ASSERT_FALSE(
        VehicleStateManager_SetState(VEHICLE_STATE_RUNNING)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );
}


/*
 * Test 17:
 * Verify the normal ignition progression:
 *
 * OFF -> ACCESSORY -> RUNNING
 */
void test_VehicleStateManager_ShouldSupportNormalStartupSequence(void)
{
    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_ACCESSORY)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_ACCESSORY,
        VehicleStateManager_GetState()
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_RUNNING)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_RUNNING,
        VehicleStateManager_GetState()
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_IsRunning()
    );
}


/*
 * Test 18:
 * Verify a complete operating cycle:
 *
 * OFF
 *  ↓
 * ACCESSORY
 *  ↓
 * RUNNING
 *  ↓
 * ACCESSORY
 *  ↓
 * OFF
 */
void test_VehicleStateManager_ShouldSupportCompleteOperatingCycle(void)
{
    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_ACCESSORY)
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_RUNNING)
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_ACCESSORY)
    );

    TEST_ASSERT_TRUE(
        VehicleStateManager_SetState(VEHICLE_STATE_OFF)
    );

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_OFF,
        VehicleStateManager_GetState()
    );

    TEST_ASSERT_FALSE(
        VehicleStateManager_IsRunning()
    );
}


/*
 * Test 19:
 * MainFunction should leave a valid state unchanged.
 */
void test_VehicleStateManager_MainFunctionShouldPreserveValidState(void)
{
    VehicleStateManager_SetState(
        VEHICLE_STATE_ACCESSORY
    );

    VehicleStateManager_MainFunction();

    TEST_ASSERT_EQUAL(
        VEHICLE_STATE_ACCESSORY,
        VehicleStateManager_GetState()
    );
}