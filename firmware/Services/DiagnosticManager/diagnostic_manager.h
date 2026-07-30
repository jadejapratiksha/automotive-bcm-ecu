#ifndef DIAGNOSTIC_MANAGER_H
#define DIAGNOSTIC_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    DTC_NONE = 0,

    DTC_BATTERY_LOW          = 0x1001U,
    DTC_BATTERY_CRITICAL     = 0x1002U,
    DTC_BATTERY_OVER_VOLTAGE = 0x1003U,
    DTC_INVALID_VEHICLE_STATE = 0x1004U

} diagnostic_code_t;

void DiagnosticManager_Init(void);
void DiagnosticManager_MainFunction(void);

bool DiagnosticManager_IsDTCActive(diagnostic_code_t dtc);

uint32_t DiagnosticManager_GetDTCMask(void);

#endif /* DIAGNOSTIC_MANAGER_H */
