#ifndef RTOS_QUEUE_H
#define RTOS_QUEUE_H

#include <stdbool.h>

#include "Drivers/CAN/can_driver.h"

/*
 * Initialize RTOS queue abstraction.
 *
 * The actual queue object is created by CubeMX.
 */
void RTOS_Queue_Init(void);

/*
 * Put one CAN message into the RX queue.
 *
 * Intended to be called from CAN RX interrupt context.
 */
bool RTOS_Queue_PushCANRx(const can_message_t *message);

/*
 * Retrieve one CAN RX message.
 *
 * Non-blocking.
 */
bool RTOS_Queue_PopCANRx(can_message_t *message);

#endif /* RTOS_QUEUE_H */
