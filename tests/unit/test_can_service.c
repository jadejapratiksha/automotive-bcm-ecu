#include "unity.h"

#include "can_service.h"
#include "can_protocol.h"

#include "mock_can_driver.h"
#include "mock_vehicle_state_manager.h"
#include "mock_door_manager.h"
#include "mock_battery_monitor.h"
#include "mock_fault_manager.h"


void setUp(void)
{
    /* Reset CANService's periodic status counter before every test. */
    CANService_Init();
}


void tearDown(void)
{
}


static can_message_t MakeMessage(uint32_t id, uint8_t dlc, uint8_t byte0)
{
    can_message_t message = {0};

    message.id = id;
    message.dlc = dlc;
    message.data[0] = byte0;

    return message;
}


static can_message_t MakeIgnitionMessage(uint8_t command)
{
    return MakeMessage(
        CAN_ID_IGNITION_COMMAND,
        CAN_IGNITION_COMMAND_DLC,
        command
    );
}


static can_message_t MakeExpectedStatusMessage(
    vehicle_state_t vehicle_state,
    door_state_t door_state,
    uint16_t battery_mv,
    uint32_t fault_mask)
{
    can_message_t message = {0};

    message.id = CAN_ID_BCM_STATUS;
    message.dlc = CAN_BCM_STATUS_DLC;

    message.data[CAN_BCM_STATUS_VEHICLE_BYTE] =
        (uint8_t)vehicle_state;

    message.data[CAN_BCM_STATUS_DOOR_BYTE] =
        (uint8_t)door_state;

    message.data[CAN_BCM_STATUS_BATTERY_LSB] =
        (uint8_t)(battery_mv & 0xFFU);

    message.data[CAN_BCM_STATUS_BATTERY_MSB] =
        (uint8_t)((battery_mv >> 8U) & 0xFFU);

    message.data[CAN_BCM_STATUS_FAULT_BYTE_0] =
        (uint8_t)(fault_mask & 0xFFU);

    message.data[CAN_BCM_STATUS_FAULT_BYTE_1] =
        (uint8_t)((fault_mask >> 8U) & 0xFFU);

    message.data[CAN_BCM_STATUS_FAULT_BYTE_2] =
        (uint8_t)((fault_mask >> 16U) & 0xFFU);

    message.data[CAN_BCM_STATUS_FAULT_BYTE_3] =
        (uint8_t)((fault_mask >> 24U) & 0xFFU);

    return message;
}


static void ExpectNoReceivedMessage(void)
{
    CAN_Driver_Receive_ExpectAnyArgsAndReturn(false);
}


static void ExpectReceivedMessage(const can_message_t *message)
{
    CAN_Driver_Receive_ExpectAnyArgsAndReturn(true);
    CAN_Driver_Receive_ReturnThruPtr_message(message);
}

static void ExpectStatusTransmission(
    vehicle_state_t vehicle_state,
    door_state_t door_state,
    uint16_t battery_mv,
    uint32_t fault_mask,
    bool send_result)
{
    /*
     * Static storage is required because CMock keeps a pointer to this
     * expected message until CANService_SendBCMStatus() is called.
     *
     * A normal local variable would become invalid when this helper returns.
     */
    static can_message_t expected;

    expected = MakeExpectedStatusMessage(
        vehicle_state,
        door_state,
        battery_mv,
        fault_mask
    );

    DoorManager_GetFrontLeftState_ExpectAndReturn(door_state);

    VehicleStateManager_GetState_ExpectAndReturn(vehicle_state);

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(battery_mv);

    FaultManager_GetActiveFaultMask_ExpectAndReturn(fault_mask);

    CAN_Driver_Send_ExpectWithArrayAndReturn(
        &expected,
        1U,
        send_result
    );
}


/* 1 */
void test_CANService_InitShouldComplete(void)
{
    CANService_Init();
    TEST_PASS();
}


/* 2 */
void test_CANService_MainFunctionShouldHandleNoReceivedMessage(void)
{
    ExpectNoReceivedMessage();
    CANService_MainFunction();
}


/* 3 */
void test_CANService_ShouldRequestOffStateForIgnitionCommand0(void)
{
    can_message_t message = MakeIgnitionMessage(CAN_IGNITION_OFF);

    ExpectReceivedMessage(&message);
    VehicleStateManager_SetState_ExpectAndReturn(VEHICLE_STATE_OFF, true);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 4 */
void test_CANService_ShouldRequestAccessoryStateForIgnitionCommand1(void)
{
    can_message_t message = MakeIgnitionMessage(CAN_IGNITION_ACCESSORY);

    ExpectReceivedMessage(&message);
    VehicleStateManager_SetState_ExpectAndReturn(
        VEHICLE_STATE_ACCESSORY,
        true
    );
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 5 */
void test_CANService_ShouldRequestRunningStateForIgnitionCommand2(void)
{
    can_message_t message = MakeIgnitionMessage(CAN_IGNITION_RUNNING);

    ExpectReceivedMessage(&message);
    VehicleStateManager_SetState_ExpectAndReturn(VEHICLE_STATE_RUNNING, true);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 6 */
void test_CANService_ShouldIgnoreIgnitionCommand3(void)
{
    can_message_t message = MakeIgnitionMessage(3U);

    ExpectReceivedMessage(&message);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 7 */
void test_CANService_ShouldIgnoreIgnitionCommand255(void)
{
    can_message_t message = MakeIgnitionMessage(0xFFU);

    ExpectReceivedMessage(&message);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 8 */
void test_CANService_ShouldIgnoreIgnitionMessageWithZeroDlc(void)
{
    can_message_t message = MakeMessage(
        CAN_ID_IGNITION_COMMAND,
        0U,
        CAN_IGNITION_OFF
    );

    ExpectReceivedMessage(&message);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 9 */
void test_CANService_ShouldIgnoreIgnitionMessageWithDlc2(void)
{
    can_message_t message = MakeMessage(
        CAN_ID_IGNITION_COMMAND,
        2U,
        CAN_IGNITION_ACCESSORY
    );

    ExpectReceivedMessage(&message);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 10 */
void test_CANService_ShouldIgnoreIgnitionMessageWithDlc8(void)
{
    can_message_t message = MakeMessage(
        CAN_ID_IGNITION_COMMAND,
        CAN_DRIVER_MAX_DATA_LENGTH,
        CAN_IGNITION_RUNNING
    );

    ExpectReceivedMessage(&message);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 11 */
void test_CANService_ShouldIgnoreUnknownCanIdZero(void)
{
    can_message_t message = MakeMessage(
        0U,
        CAN_IGNITION_COMMAND_DLC,
        CAN_IGNITION_ACCESSORY
    );

    ExpectReceivedMessage(&message);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 12 */
void test_CANService_ShouldIgnoreUnknownStandardCanId(void)
{
    can_message_t message = MakeMessage(
        0x7FFU,
        CAN_IGNITION_COMMAND_DLC,
        CAN_IGNITION_RUNNING
    );

    ExpectReceivedMessage(&message);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 13 */
void test_CANService_ShouldIgnoreReceivedBcmStatusFrame(void)
{
    can_message_t message = MakeMessage(
        CAN_ID_BCM_STATUS,
        CAN_BCM_STATUS_DLC,
        (uint8_t)VEHICLE_STATE_RUNNING
    );

    ExpectReceivedMessage(&message);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 14 */
void test_CANService_ShouldProcessTwoValidReceivedMessages(void)
{
    can_message_t message1 = MakeIgnitionMessage(CAN_IGNITION_ACCESSORY);
    can_message_t message2 = MakeIgnitionMessage(CAN_IGNITION_RUNNING);

    ExpectReceivedMessage(&message1);
    VehicleStateManager_SetState_ExpectAndReturn(
        VEHICLE_STATE_ACCESSORY,
        true
    );

    ExpectReceivedMessage(&message2);
    VehicleStateManager_SetState_ExpectAndReturn(VEHICLE_STATE_RUNNING, true);

    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 15 */
void test_CANService_ShouldContinueAfterInvalidThenProcessValidMessage(void)
{
    can_message_t invalid = MakeIgnitionMessage(99U);
    can_message_t valid = MakeIgnitionMessage(CAN_IGNITION_OFF);

    ExpectReceivedMessage(&invalid);
    ExpectReceivedMessage(&valid);
    VehicleStateManager_SetState_ExpectAndReturn(VEHICLE_STATE_OFF, true);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 16 */
void test_CANService_ShouldProcessValidThenIgnoreInvalidMessage(void)
{
    can_message_t valid = MakeIgnitionMessage(CAN_IGNITION_RUNNING);
    can_message_t invalid = MakeMessage(
        CAN_ID_IGNITION_COMMAND,
        0U,
        CAN_IGNITION_OFF
    );

    ExpectReceivedMessage(&valid);
    VehicleStateManager_SetState_ExpectAndReturn(VEHICLE_STATE_RUNNING, true);
    ExpectReceivedMessage(&invalid);
    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 17 */
void test_CANService_ShouldProcessOffAccessoryRunningSequence(void)
{
    can_message_t off = MakeIgnitionMessage(CAN_IGNITION_OFF);
    can_message_t accessory = MakeIgnitionMessage(CAN_IGNITION_ACCESSORY);
    can_message_t running = MakeIgnitionMessage(CAN_IGNITION_RUNNING);

    ExpectReceivedMessage(&off);
    VehicleStateManager_SetState_ExpectAndReturn(VEHICLE_STATE_OFF, true);

    ExpectReceivedMessage(&accessory);
    VehicleStateManager_SetState_ExpectAndReturn(
        VEHICLE_STATE_ACCESSORY,
        true
    );

    ExpectReceivedMessage(&running);
    VehicleStateManager_SetState_ExpectAndReturn(VEHICLE_STATE_RUNNING, true);

    ExpectNoReceivedMessage();

    CANService_MainFunction();
}


/* 18 */
void test_CANService_SendBCMStatusShouldBuildCorrectMessage(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_RUNNING,
        DOOR_STATE_OPEN,
        0x1234U,
        0x78563412UL,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 19 */
void test_CANService_SendBCMStatusShouldReturnFalseWhenDriverFails(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_OFF,
        DOOR_STATE_CLOSED,
        12000U,
        0U,
        false
    );

    TEST_ASSERT_FALSE(CANService_SendBCMStatus());
}


/* 20 */
void test_CANService_SendBCMStatusShouldEncodeAllZeroValues(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_OFF,
        DOOR_STATE_CLOSED,
        0U,
        0U,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 21 */
void test_CANService_SendBCMStatusShouldEncodeMaximumBatteryAndFaultValues(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_RUNNING,
        DOOR_STATE_OPEN,
        0xFFFFU,
        0xFFFFFFFFUL,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 22 */
void test_CANService_ShouldEncodeBatteryValue00FFLittleEndian(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_OFF,
        DOOR_STATE_CLOSED,
        0x00FFU,
        0U,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 23 */
void test_CANService_ShouldEncodeBatteryValueFF00LittleEndian(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_ACCESSORY,
        DOOR_STATE_CLOSED,
        0xFF00U,
        0U,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 24 */
void test_CANService_ShouldEncodeFaultMaskLowByte(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_OFF,
        DOOR_STATE_CLOSED,
        12000U,
        0x000000A5UL,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 25 */
void test_CANService_ShouldEncodeFaultMaskSecondByte(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_OFF,
        DOOR_STATE_CLOSED,
        12000U,
        0x00005A00UL,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 26 */
void test_CANService_ShouldEncodeFaultMaskThirdByte(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_OFF,
        DOOR_STATE_CLOSED,
        12000U,
        0x00C30000UL,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 27 */
void test_CANService_ShouldEncodeFaultMaskHighByte(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_OFF,
        DOOR_STATE_CLOSED,
        12000U,
        0x7E000000UL,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 28 */
void test_CANService_ShouldEncodeOpenDoorAndAccessoryState(void)
{
    ExpectStatusTransmission(
        VEHICLE_STATE_ACCESSORY,
        DOOR_STATE_OPEN,
        13500U,
        0x00000003UL,
        true
    );

    TEST_ASSERT_TRUE(CANService_SendBCMStatus());
}


/* 29 */
void test_CANService_MainFunctionShouldNotSendStatusBeforeTenthCall(void)
{
    uint8_t call_index;

    for (call_index = 0U; call_index < 9U; call_index++)
    {
        ExpectNoReceivedMessage();
    }

    for (call_index = 0U; call_index < 9U; call_index++)
    {
        CANService_MainFunction();
    }
}


/* 30 */
void test_CANService_MainFunctionShouldSendStatusOnTenthCall(void)
{
    uint8_t call_index;

    for (call_index = 0U; call_index < 10U; call_index++)
    {
        ExpectNoReceivedMessage();
    }

    ExpectStatusTransmission(
        VEHICLE_STATE_RUNNING,
        DOOR_STATE_OPEN,
        14200U,
        0x00000005UL,
        true
    );

    for (call_index = 0U; call_index < 10U; call_index++)
    {
        CANService_MainFunction();
    }
}