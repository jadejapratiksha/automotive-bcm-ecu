#include "Drivers/CAN/can_driver.h"
#include "main.h"
#include "cmsis_os2.h"
#include "RTOS/rtos_queues.h"

extern CAN_HandleTypeDef hcan1;


void CAN_Driver_Init(void)
{
    CAN_FilterTypeDef filter_config;


    /*
     * Accept all standard CAN messages.
     */
    filter_config.FilterBank = 0U;
    filter_config.FilterMode = CAN_FILTERMODE_IDMASK;
    filter_config.FilterScale = CAN_FILTERSCALE_32BIT;

    filter_config.FilterIdHigh = 0U;
    filter_config.FilterIdLow = 0U;

    filter_config.FilterMaskIdHigh = 0U;
    filter_config.FilterMaskIdLow = 0U;

    filter_config.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter_config.FilterActivation = ENABLE;

    filter_config.SlaveStartFilterBank = 14U;

    if (HAL_CAN_ConfigFilter(&hcan1,
                             &filter_config) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }

    /*
     * Enable interrupt when a CAN message arrives in FIFO0.
     */
    if (HAL_CAN_ActivateNotification(
            &hcan1,
            CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }
}

bool CAN_Driver_Send(const can_message_t *message)
{
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;

    if (message == NULL)
    {
        return false;
    }

    if (message->dlc > CAN_DRIVER_MAX_DATA_LENGTH)
    {
        return false;
    }

    if (message->id > 0x7FFU)
    {
        return false;
    }

    tx_header.StdId = message->id;
    tx_header.ExtId = 0U;

    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;

    tx_header.DLC = message->dlc;

    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan1,
                             &tx_header,
                             (uint8_t *)message->data,
                             &tx_mailbox) != HAL_OK)
    {
        return false;
    }

    return true;
}

bool CAN_Driver_Receive(can_message_t *message)
{
	return RTOS_Queue_PopCANRx(message);
}

void CAN_Driver_RxInterruptHandler(void)
{
    CAN_RxHeaderTypeDef rx_header;
    can_message_t message;

    if (HAL_CAN_GetRxMessage(
            &hcan1,
            CAN_RX_FIFO0,
            &rx_header,
            message.data) != HAL_OK)
    {
        return;
    }

    /*
     * Standard CAN IDs only.
     */
    if (rx_header.IDE != CAN_ID_STD)
    {
        return;
    }

    message.id = rx_header.StdId;
    message.dlc = rx_header.DLC;

    /*
     * Transfer received CAN frame from ISR context
     * into the FreeRTOS CAN RX queue.
     */
    (void)RTOS_Queue_PushCANRx(&message);
}
