#ifndef SIM_INTERFACE_H
#define SIM_INTERFACE_H

#include <stdint.h>

/*
 * Initialize the complete simulation interface.
 */
void SimInterface_Init(void);

/*
 * Supply one received UART byte to the command-line parser.
 *
 * Call this from HAL_UART_RxCpltCallback().
 */
void SimInterface_ProcessByte(uint8_t received_byte);

/*
 * Process a completed command.
 *
 * Call this periodically from an RTOS task.
 */
void SimInterface_MainFunction(void);

#endif /* SIM_INTERFACE_H */
