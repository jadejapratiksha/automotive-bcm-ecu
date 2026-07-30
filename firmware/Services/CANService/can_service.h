#ifndef CAN_SERVICE_H
#define CAN_SERVICE_H

#include <stdint.h>
#include <stdbool.h>

void CANService_Init(void);
void CANService_MainFunction(void);

/*
 * Send current BCM status over CAN.
 */
bool CANService_SendBCMStatus(void);

#endif /* CAN_SERVICE_H */
