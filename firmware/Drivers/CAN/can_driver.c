#include "Drivers/CAN/can_driver.h"
#include "main.h"
#include "cmsis_os2.h"
#include "RTOS/rtos_queues.h"

extern CAN_HandleTypeDef hcan1;


extern UART_HandleTypeDef huart2;

static void CAN_DebugPrint(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    (void)HAL_UART_Transmit(
        &huart2,
        (uint8_t *)text,
        (uint16_t)strlen(text),
        100U);
}

void CAN_Driver_Init(void)
{
	 CAN_FilterTypeDef filter_config = {0};

	 CAN_DebugPrint("CAN INIT START\r\n");


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

    CAN_DebugPrint("CAN FILTER CONFIG\r\n");

    if (HAL_CAN_ConfigFilter(&hcan1,
                             &filter_config) != HAL_OK)
    {
    	CAN_DebugPrint("CAN FILTER FAILED\r\n");
        Error_Handler();
    }
    CAN_DebugPrint("CAN FILTER OK\r\n");
    CAN_DebugPrint("CAN STARTING\r\n");


    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
    	CAN_DebugPrint("CAN START FAILED\r\n");
        Error_Handler();
    }
    CAN_DebugPrint("CAN START OK\r\n");

    /*
     * Enable interrupt when a CAN message arrives in FIFO0.
     */
    if (HAL_CAN_ActivateNotification(
            &hcan1,
            CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
    	CAN_DebugPrint("CAN NOTIFICATION FAILED\r\n");
        Error_Handler();
    }

    CAN_DebugPrint("CAN NOTIFICATION OK\r\n");
    CAN_DebugPrint("CAN INIT COMPLETE\r\n");
}

bool CAN_Driver_Send(const can_message_t *message)
{
    CAN_TxHeaderTypeDef tx_header = {0};
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
    CAN_RxHeaderTypeDef rx_header= {0};
    can_message_t message= {0};

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
