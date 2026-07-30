#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Battery voltage condition.
 */
typedef enum
{
    BATTERY_STATE_NORMAL = 0,
    BATTERY_STATE_LOW,
    BATTERY_STATE_CRITICAL,
    BATTERY_STATE_OVER_VOLTAGE,

    BATTERY_STATE_MAX
} battery_state_t;

/*
 * Initialize the Battery Monitor.
 */
void BatteryMonitor_Init(void);

/*
 * Periodic battery-monitoring logic.
 */
void BatteryMonitor_MainFunction(void);

/*
 * Supply the latest measured battery voltage.
 *
 * Example:
 * 12000 mV = 12.0 V
 * 14500 mV = 14.5 V
 */
void BatteryMonitor_UpdateVoltageMv(uint16_t voltage_mv);

/*
 * Return the most recently measured voltage.
 */
uint16_t BatteryMonitor_GetVoltageMv(void);

/*
 * Return the current battery condition.
 */
battery_state_t BatteryMonitor_GetState(void);

/*
 * Return true when the voltage is low or critical.
 */
bool BatteryMonitor_IsLowVoltage(void);

#endif /* BATTERY_MONITOR_H */