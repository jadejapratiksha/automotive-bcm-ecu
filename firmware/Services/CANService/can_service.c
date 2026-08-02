#include "Services/CANService/can_service.h"
#include "Services/CANService/can_protocol.h"

#include "Drivers/CAN/can_driver.h"
#include "App/VehicleStateManager/vehicle_state_manager.h"
#include "App/DoorManager/door_manager.h"
#include "App/BatteryMonitor/battery_monitor.h"
#include "Services/FaultManager/fault_manager.h"


/*
 * Module-level counter so CANService_Init() can reset it.
 * This makes initialization deterministic in production and unit tests.
 */
static uint8_t status_counter = 0U;


static void CANService_ProcessRx(void)
{
    can_message_t rx_message = {0};

    while (CAN_Driver_Receive(&rx_message) == true)
    {
        switch (rx_message.id)
        {
            case CAN_ID_IGNITION_COMMAND:
                if (rx_message.dlc == CAN_IGNITION_COMMAND_DLC)
                {
                    switch (rx_message.data[0])
                    {
                        case CAN_IGNITION_OFF:
                            (void)VehicleStateManager_SetState(
                                VEHICLE_STATE_OFF
                            );
                            break;

                        case CAN_IGNITION_ACCESSORY:
                            (void)VehicleStateManager_SetState(
                                VEHICLE_STATE_ACCESSORY
                            );
                            break;

                        case CAN_IGNITION_RUNNING:
                            (void)VehicleStateManager_SetState(
                                VEHICLE_STATE_RUNNING
                            );
                            break;

                        default:
                            /* Ignore unsupported ignition commands. */
                            break;
                    }
                }
                break;

            default:
                /* Ignore unsupported CAN identifiers. */
                break;
        }
    }
}


bool CANService_SendBCMStatus(void)
{
    can_message_t tx_message = {0};
    door_state_t door_state;
    vehicle_state_t vehicle_state;
    uint16_t battery_mv;
    uint32_t fault_mask;

    door_state = DoorManager_GetFrontLeftState();
    vehicle_state = VehicleStateManager_GetState();
    battery_mv = BatteryMonitor_GetVoltageMv();
    fault_mask = FaultManager_GetActiveFaultMask();

    tx_message.id = CAN_ID_BCM_STATUS;
    tx_message.dlc = CAN_BCM_STATUS_DLC;

    tx_message.data[CAN_BCM_STATUS_VEHICLE_BYTE] =
        (uint8_t)vehicle_state;

    tx_message.data[CAN_BCM_STATUS_DOOR_BYTE] =
        (uint8_t)door_state;

    tx_message.data[CAN_BCM_STATUS_BATTERY_LSB] =
        (uint8_t)(battery_mv & 0xFFU);

    tx_message.data[CAN_BCM_STATUS_BATTERY_MSB] =
        (uint8_t)((battery_mv >> 8U) & 0xFFU);

    tx_message.data[CAN_BCM_STATUS_FAULT_BYTE_0] =
        (uint8_t)(fault_mask & 0xFFU);

    tx_message.data[CAN_BCM_STATUS_FAULT_BYTE_1] =
        (uint8_t)((fault_mask >> 8U) & 0xFFU);

    tx_message.data[CAN_BCM_STATUS_FAULT_BYTE_2] =
        (uint8_t)((fault_mask >> 16U) & 0xFFU);

    tx_message.data[CAN_BCM_STATUS_FAULT_BYTE_3] =
        (uint8_t)((fault_mask >> 24U) & 0xFFU);

    return CAN_Driver_Send(&tx_message);
}


void CANService_Init(void)
{
    status_counter = 0U;
}


void CANService_MainFunction(void)
{
    CANService_ProcessRx();

    status_counter++;

    if (status_counter >= 10U)
    {
        status_counter = 0U;
        (void)CANService_SendBCMStatus();
    }
}
