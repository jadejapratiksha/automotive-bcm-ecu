#include "Services/FaultManager/fault_manager.h"

#include "App/BatteryMonitor/battery_monitor.h"
#include "App/VehicleStateManager/vehicle_state_manager.h"

/*
 * Each bit represents one fault.
 *
 * Bit 0 = battery low
 * Bit 1 = battery critical
 * Bit 2 = battery overvoltage
 * Bit 3 = invalid vehicle state
 */
static uint32_t active_fault_mask;

static bool FaultManager_IsValidFault(fault_id_t fault_id)
{
    return (fault_id < FAULT_ID_MAX);
}

void FaultManager_Init(void)
{
    active_fault_mask = 0U;
}

void FaultManager_SetFault(fault_id_t fault_id)
{
    if (FaultManager_IsValidFault(fault_id) == true)
    {
        active_fault_mask |= (1UL << (uint32_t)fault_id);
    }
}

void FaultManager_ClearFault(fault_id_t fault_id)
{
    if (FaultManager_IsValidFault(fault_id) == true)
    {
        active_fault_mask &= ~(1UL << (uint32_t)fault_id);
    }
}

bool FaultManager_IsFaultActive(fault_id_t fault_id)
{
    bool fault_active = false;

    if (FaultManager_IsValidFault(fault_id) == true)
    {
        fault_active =
            ((active_fault_mask &
              (1UL << (uint32_t)fault_id)) != 0U);
    }

    return fault_active;
}

bool FaultManager_IsAnyFaultActive(void)
{
    return (active_fault_mask != 0U);
}

uint32_t FaultManager_GetActiveFaultMask(void)
{
    return active_fault_mask;
}

void FaultManager_MainFunction(void)
{
    battery_state_t battery_state;
    vehicle_state_t vehicle_state;

    battery_state = BatteryMonitor_GetState();
    vehicle_state = VehicleStateManager_GetState();

    /*
     * Battery low fault
     */
    if (battery_state == BATTERY_STATE_LOW)
    {
        FaultManager_SetFault(FAULT_ID_BATTERY_LOW);
    }
    else
    {
        FaultManager_ClearFault(FAULT_ID_BATTERY_LOW);
    }

    /*
     * Battery critical fault
     */
    if (battery_state == BATTERY_STATE_CRITICAL)
    {
        FaultManager_SetFault(FAULT_ID_BATTERY_CRITICAL);
    }
    else
    {
        FaultManager_ClearFault(FAULT_ID_BATTERY_CRITICAL);
    }

    /*
     * Battery overvoltage fault
     */
    if (battery_state == BATTERY_STATE_OVER_VOLTAGE)
    {
        FaultManager_SetFault(FAULT_ID_BATTERY_OVER_VOLTAGE);
    }
    else
    {
        FaultManager_ClearFault(FAULT_ID_BATTERY_OVER_VOLTAGE);
    }

    /*
     * Invalid vehicle-state protection
     */
    if (vehicle_state >= VEHICLE_STATE_MAX)
    {
        FaultManager_SetFault(FAULT_ID_INVALID_VEHICLE_STATE);
    }
    else
    {
        FaultManager_ClearFault(FAULT_ID_INVALID_VEHICLE_STATE);
    }
}