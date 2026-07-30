#include "Services/CANService/can_service.h"

#include "Drivers/CAN/can_driver.h"
#include "App/VehicleStateManager/vehicle_state_manager.h"
#include "App/DoorManager/door_manager.h"
#include "App/BatteryMonitor/battery_monitor.h"
#include "Services/FaultManager/fault_manager.h"

/*
 * Example CAN IDs from our BCM database.
 */
#define CAN_ID_IGNITION_COMMAND     (0x100U)
#define CAN_ID_BCM_STATUS           (0x200U)

static void CANService_ProcessRx(void)
{
    can_message_t rx_message;

    while (CAN_Driver_Receive(&rx_message) == true)
    {
        switch (rx_message.id)
        {
            case CAN_ID_IGNITION_COMMAND:

                if (rx_message.dlc >= 1U)
                {
                    switch (rx_message.data[0])
                    {
                        case 0U:
                            (void)VehicleStateManager_SetState(
                                    VEHICLE_STATE_OFF);
                            break;

                        case 1U:
                            (void)VehicleStateManager_SetState(
                                    VEHICLE_STATE_ACCESSORY);
                            break;

                        case 2U:
                            (void)VehicleStateManager_SetState(
                                    VEHICLE_STATE_RUNNING);
                            break;

                        default:
                            break;
                    }
                }

                break;

            default:
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
    tx_message.dlc = 8U;

    tx_message.data[0] = (uint8_t)vehicle_state;
    tx_message.data[1] = (uint8_t)door_state;

    tx_message.data[2] = (uint8_t)(battery_mv & 0xFFU);
    tx_message.data[3] = (uint8_t)((battery_mv >> 8U) & 0xFFU);

    tx_message.data[4] = (uint8_t)(fault_mask & 0xFFU);
    tx_message.data[5] = (uint8_t)((fault_mask >> 8U) & 0xFFU);
    tx_message.data[6] = (uint8_t)((fault_mask >> 16U) & 0xFFU);
    tx_message.data[7] = (uint8_t)((fault_mask >> 24U) & 0xFFU);

    return CAN_Driver_Send(&tx_message);
}

void CANService_Init(void)
{
    /* Driver initialization is handled separately. */
}

void CANService_MainFunction(void)
{

	static uint8_t status_counter = 0U;

	CANService_ProcessRx();

	status_counter++;

	if (status_counter >= 10U)
	{
		status_counter = 0U;

		(void)CANService_SendBCMStatus();
	}

}
