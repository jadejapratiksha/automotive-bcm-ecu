#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define CAN_DRIVER_MAX_DATA_LENGTH    (8U)

typedef struct
{
    uint32_t id;
    uint8_t dlc;
    uint8_t data[CAN_DRIVER_MAX_DATA_LENGTH];

} can_message_t;


void CAN_Driver_Init(void);

bool CAN_Driver_Send(const can_message_t *message);

/*
 * Returns next received CAN message from software RX buffer.
 */
bool CAN_Driver_Receive(can_message_t *message);

/*
 * Called from HAL CAN RX interrupt callback.
 */
void CAN_Driver_RxInterruptHandler(void);

#endif /* CAN_DRIVER_H */
