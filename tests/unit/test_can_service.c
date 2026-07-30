#include "unity.h"

#include "can_service.h"

#include "mock_can_driver.h"
#include "mock_vehicle_state_manager.h"
#include "mock_door_manager.h"
#include "mock_battery_monitor.h"
#include "mock_fault_manager.h"


void setUp(void)
{
}


void tearDown(void)
{
}


/*
 * Helper:
 * Create an ignition command CAN message.
 *
 * ID 0x100
 * Byte 0:
 *   0 = OFF
 *   1 = ACCESSORY
 *   2 = RUNNING
 */
static can_message_t MakeIgnitionMessage(uint8_t command)
{
    can_message_t message = {0};

    message.id = 0x100U;
    message.dlc = 1U;
    message.data[0] = command;

    return message;
}


/*
 * Test 1:
 * Init should complete without requiring driver initialization.
 */
void test_CANService_InitShouldComplete(void)
{
    CANService_Init();

    TEST_PASS();
}


/*
 * Test 2:
 * No RX message available.
 *
 * CANService should simply return without changing state.
 */
void test_CANService_MainFunctionShouldHandleNoReceivedMessage(void)
{
    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);

    CANService_MainFunction();
}


/*
 * Test 3:
 * Ignition command 0 should request OFF state.
 */
void test_CANService_ShouldRequestOffStateForIgnitionCommand0(void)
{
    can_message_t message = MakeIgnitionMessage(0U);

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(&message);

    VehicleStateManager_SetState_ExpectAndReturn(
        VEHICLE_STATE_OFF,
        true
    );

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);

    CANService_MainFunction();
}


/*
 * Test 4:
 * Ignition command 1 should request ACCESSORY state.
 */
void test_CANService_ShouldRequestAccessoryStateForIgnitionCommand1(void)
{
    can_message_t message = MakeIgnitionMessage(1U);

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(&message);

    VehicleStateManager_SetState_ExpectAndReturn(
        VEHICLE_STATE_ACCESSORY,
        true
    );

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);

    CANService_MainFunction();
}


/*
 * Test 5:
 * Ignition command 2 should request RUNNING state.
 */
void test_CANService_ShouldRequestRunningStateForIgnitionCommand2(void)
{
    can_message_t message = MakeIgnitionMessage(2U);

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(&message);

    VehicleStateManager_SetState_ExpectAndReturn(
        VEHICLE_STATE_RUNNING,
        true
    );

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);

    CANService_MainFunction();
}


/*
 * Test 6:
 * Invalid ignition command should be ignored.
 */
void test_CANService_ShouldIgnoreInvalidIgnitionCommand(void)
{
    can_message_t message = MakeIgnitionMessage(99U);

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(&message);

    /*
     * No VehicleStateManager expectation here.
     * If CANService calls it, CMock will fail the test.
     */

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);

    CANService_MainFunction();
}


/*
 * Test 7:
 * Ignition frame with DLC 0 should be ignored.
 */
void test_CANService_ShouldIgnoreIgnitionMessageWithZeroDlc(void)
{
    can_message_t message = {0};

    message.id = 0x100U;
    message.dlc = 0U;

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(&message);

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);

    CANService_MainFunction();
}


/*
 * Test 8:
 * Unknown CAN IDs should be ignored.
 */
void test_CANService_ShouldIgnoreUnknownCanId(void)
{
    can_message_t message = {0};

    message.id = 0x555U;
    message.dlc = 1U;
    message.data[0] = 1U;

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(&message);

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);

    CANService_MainFunction();
}


/*
 * Test 9:
 * CANService should process multiple queued RX messages
 * in a single MainFunction call.
 */
void test_CANService_ShouldProcessMultipleReceivedMessages(void)
{
    can_message_t message1 = MakeIgnitionMessage(1U);
    can_message_t message2 = MakeIgnitionMessage(2U);

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(&message1);

    VehicleStateManager_SetState_ExpectAndReturn(
        VEHICLE_STATE_ACCESSORY,
        true
    );

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(&message2);

    VehicleStateManager_SetState_ExpectAndReturn(
        VEHICLE_STATE_RUNNING,
        true
    );

    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);

    CANService_MainFunction();
}


/*
 * Test 10:
 * SendBCMStatus should build an 8-byte BCM status frame.
 *
 * Expected format:
 *
 * byte 0 = vehicle state
 * byte 1 = door state
 * byte 2 = battery low byte
 * byte 3 = battery high byte
 * byte 4 = fault mask byte 0
 * byte 5 = fault mask byte 1
 * byte 6 = fault mask byte 2
 * byte 7 = fault mask byte 3
 */
void test_CANService_SendBCMStatusShouldBuildCorrectMessage(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_OPEN
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_RUNNING
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        0x1234U
    );

    FaultManager_GetActiveFaultMask_ExpectAndReturn(
        0x78563412UL
    );

    can_message_t expected = {0};

    expected.id = 0x200U;
    expected.dlc = 8U;

    expected.data[0] = (uint8_t)VEHICLE_STATE_RUNNING;
    expected.data[1] = (uint8_t)DOOR_STATE_OPEN;

    expected.data[2] = 0x34U;
    expected.data[3] = 0x12U;

    expected.data[4] = 0x12U;
    expected.data[5] = 0x34U;
    expected.data[6] = 0x56U;
    expected.data[7] = 0x78U;

    CAN_Driver_Send_ExpectWithArrayAndReturn(
        &expected,
        1U,
        true
    );

    TEST_ASSERT_TRUE(
        CANService_SendBCMStatus()
    );
}


/*
 * Test 11:
 * SendBCMStatus should return false if CAN driver
 * transmission fails.
 */
void test_CANService_SendBCMStatusShouldReturnFalseWhenDriverSendFails(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_CLOSED
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        12000U
    );

    FaultManager_GetActiveFaultMask_ExpectAndReturn(
        0U
    );

    CAN_Driver_Send_IgnoreAndReturn(false);

    TEST_ASSERT_FALSE(
        CANService_SendBCMStatus()
    );
}


/*
 * Test 12:
 * Verify battery voltage is transmitted little-endian.
 *
 * 12000 decimal = 0x2EE0
 *
 * byte 2 = 0xE0
 * byte 3 = 0x2E
 */
void test_CANService_ShouldEncodeBatteryVoltageLittleEndian(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_CLOSED
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_OFF
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        12000U
    );

    FaultManager_GetActiveFaultMask_ExpectAndReturn(
        0U
    );

    can_message_t expected = {0};

    expected.id = 0x200U;
    expected.dlc = 8U;

    expected.data[0] = (uint8_t)VEHICLE_STATE_OFF;
    expected.data[1] = (uint8_t)DOOR_STATE_CLOSED;

    expected.data[2] = 0xE0U;
    expected.data[3] = 0x2EU;

    expected.data[4] = 0U;
    expected.data[5] = 0U;
    expected.data[6] = 0U;
    expected.data[7] = 0U;

    CAN_Driver_Send_ExpectWithArrayAndReturn(
        &expected,
        1U,
        true
    );

    TEST_ASSERT_TRUE(
        CANService_SendBCMStatus()
    );
}


/*
 * Test 13:
 * Verify 32-bit fault mask byte ordering.
 */
void test_CANService_ShouldEncodeFaultMaskLittleEndian(void)
{
    DoorManager_GetFrontLeftState_ExpectAndReturn(
        DOOR_STATE_CLOSED
    );

    VehicleStateManager_GetState_ExpectAndReturn(
        VEHICLE_STATE_ACCESSORY
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        12000U
    );

    FaultManager_GetActiveFaultMask_ExpectAndReturn(
        0xA1B2C3D4UL
    );

    can_message_t expected = {0};

    expected.id = 0x200U;
    expected.dlc = 8U;

    expected.data[0] = (uint8_t)VEHICLE_STATE_ACCESSORY;
    expected.data[1] = (uint8_t)DOOR_STATE_CLOSED;

    expected.data[2] = 0xE0U;
    expected.data[3] = 0x2EU;

    expected.data[4] = 0xD4U;
    expected.data[5] = 0xC3U;
    expected.data[6] = 0xB2U;
    expected.data[7] = 0xA1U;

    CAN_Driver_Send_ExpectWithArrayAndReturn(
        &expected,
        1U,
        true
    );

    TEST_ASSERT_TRUE(
        CANService_SendBCMStatus()
    );
}