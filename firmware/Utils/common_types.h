#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Common return status used across firmware modules.
 */
typedef enum
{
    STATUS_OK = 0,
    STATUS_ERROR,
    STATUS_BUSY,
    STATUS_TIMEOUT,
    STATUS_INVALID_PARAM,
    STATUS_NOT_AVAILABLE
} status_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif /* COMMON_TYPES_H */
