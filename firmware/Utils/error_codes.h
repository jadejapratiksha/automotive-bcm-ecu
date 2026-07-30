#ifndef ERROR_CODES_H
#define ERROR_CODES_H

/*
 * Generic firmware error codes.
 *
 * These are software-level errors.
 * They are different from vehicle diagnostic trouble codes (DTCs).
 */
typedef enum
{
    ERROR_CODE_NONE = 0,

    ERROR_CODE_INVALID_PARAMETER,

    ERROR_CODE_GPIO,
    ERROR_CODE_ADC_START,
    ERROR_CODE_ADC_TIMEOUT,
    ERROR_CODE_ADC_READ,

    ERROR_CODE_CAN_INIT,
    ERROR_CODE_CAN_TX,
    ERROR_CODE_CAN_RX,
    ERROR_CODE_CAN_QUEUE_FULL,

    ERROR_CODE_INVALID_VEHICLE_STATE,

    ERROR_CODE_RING_BUFFER_FULL,
    ERROR_CODE_RING_BUFFER_EMPTY,

    ERROR_CODE_MAX

} error_code_t;

#endif /* ERROR_CODES_H */
