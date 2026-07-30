#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    FAULT_ID_BATTERY_LOW = 0,
    FAULT_ID_BATTERY_CRITICAL,
    FAULT_ID_BATTERY_OVER_VOLTAGE,
    FAULT_ID_INVALID_VEHICLE_STATE,

    FAULT_ID_MAX
} fault_id_t;

void FaultManager_Init(void);

void FaultManager_MainFunction(void);

void FaultManager_SetFault(fault_id_t fault_id);

void FaultManager_ClearFault(fault_id_t fault_id);

bool FaultManager_IsFaultActive(fault_id_t fault_id);

bool FaultManager_IsAnyFaultActive(void);

uint32_t FaultManager_GetActiveFaultMask(void);

#endif /* FAULT_MANAGER_H */