#ifndef SIM_TRANSPORT_UART_H
#define SIM_TRANSPORT_UART_H

#include <stdint.h>
#include "main.h"

/*
 * Initialize the USART2 simulation transport.
 *
 * This starts interrupt-based reception of one byte at a time.
 */
void SimTransportUart_Init(void);

/*
 * Send a null-terminated string through USART2.
 */
void SimTransportUart_SendString(const char *message);

/*
 * Send a null-terminated string followed by CRLF.
 */
void SimTransportUart_SendLine(const char *message);

/*
 * Return the most recently received UART byte.
 *
 * This should be called from HAL_UART_RxCpltCallback().
 */
uint8_t SimTransportUart_GetReceivedByte(void);

/*
 * Restart interrupt-based reception for the next byte.
 *
 * This should be called after processing the current received byte.
 */
HAL_StatusTypeDef SimTransportUart_ReceiveNextByte(void);

#endif /* SIM_TRANSPORT_UART_H */
