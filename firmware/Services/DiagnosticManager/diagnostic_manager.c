#include "Services/DiagnosticManager/diagnostic_manager.h"

#include "Services/FaultManager/fault_manager.h"
#include "Services/EventLogger/event_logger.h"
#include "App/BatteryMonitor/battery_monitor.h"

static uint32_t diagnostic_mask;
static uint32_t previous_mask;

static void DiagnosticManager_SetBit(uint8_t bit)
{
    diagnostic_mask |= (1UL << bit);
}

static void DiagnosticManager_ClearBit(uint8_t bit)
{
    diagnostic_mask &= ~(1UL << bit);
}

void DiagnosticManager_Init(void)
{
    diagnostic_mask = 0U;
    previous_mask = 0U;
}

void DiagnosticManager_MainFunction(void)
{

    uint32_t current_mask;

    diagnostic_mask = 0U;

    if (FaultManager_IsFaultActive(FAULT_ID_BATTERY_LOW))
    {
        DiagnosticManager_SetBit(0U);
    }

    if (FaultManager_IsFaultActive(FAULT_ID_BATTERY_CRITICAL))
    {
        DiagnosticManager_SetBit(1U);
    }

    if (FaultManager_IsFaultActive(FAULT_ID_BATTERY_OVER_VOLTAGE))
    {
        DiagnosticManager_SetBit(2U);
    }

    if (FaultManager_IsFaultActive(FAULT_ID_INVALID_VEHICLE_STATE))
    {
        DiagnosticManager_SetBit(3U);
    }

    current_mask = diagnostic_mask;

    /*
     * Log only newly appearing diagnostic conditions.
     */
    if ((current_mask & (1UL << 0U)) &&
        !(previous_mask & (1UL << 0U)))
    {
        EventLogger_Log(EVENT_BATTERY_LOW,
                        BatteryMonitor_GetVoltageMv());
    }

    if ((current_mask & (1UL << 1U)) &&
        !(previous_mask & (1UL << 1U)))
    {
        EventLogger_Log(EVENT_BATTERY_CRITICAL,
                        BatteryMonitor_GetVoltageMv());
    }

    if ((current_mask & (1UL << 2U)) &&
        !(previous_mask & (1UL << 2U)))
    {
        EventLogger_Log(EVENT_BATTERY_OVER_VOLTAGE,
                        BatteryMonitor_GetVoltageMv());
    }

    previous_mask = current_mask;
}

bool DiagnosticManager_IsDTCActive(diagnostic_code_t dtc)
{
    switch (dtc)
    {
        case DTC_BATTERY_LOW:
            return ((diagnostic_mask & (1UL << 0U)) != 0U);

        case DTC_BATTERY_CRITICAL:
            return ((diagnostic_mask & (1UL << 1U)) != 0U);

        case DTC_BATTERY_OVER_VOLTAGE:
            return ((diagnostic_mask & (1UL << 2U)) != 0U);

        case DTC_INVALID_VEHICLE_STATE:
            return ((diagnostic_mask & (1UL << 3U)) != 0U);

        default:
            return false;
    }
}

uint32_t DiagnosticManager_GetDTCMask(void)
{
    return diagnostic_mask;
}
