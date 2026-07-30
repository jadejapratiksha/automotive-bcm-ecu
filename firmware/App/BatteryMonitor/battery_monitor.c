#include "App/BatteryMonitor/battery_monitor.h"
#include "Drivers/ADC/adc_driver.h"

/*
 * Automotive 12 V battery thresholds.
 *
 * These values are simplified project thresholds.
 */
#define BATTERY_CRITICAL_ENTER_MV      (9000U)
#define BATTERY_CRITICAL_EXIT_MV       (9500U)

#define BATTERY_LOW_ENTER_MV          (11000U)
#define BATTERY_LOW_EXIT_MV           (11500U)

#define BATTERY_OVER_VOLTAGE_ENTER_MV (15500U)
#define BATTERY_OVER_VOLTAGE_EXIT_MV  (15000U)

#define BATTERY_DEFAULT_VOLTAGE_MV    (12000U)

static uint16_t battery_voltage_mv;
static battery_state_t battery_state;

void BatteryMonitor_Init(void)
{
    battery_voltage_mv = BATTERY_DEFAULT_VOLTAGE_MV;
    battery_state = BATTERY_STATE_NORMAL;
}

void BatteryMonitor_UpdateVoltageMv(uint16_t voltage_mv)
{
    battery_voltage_mv = voltage_mv;
}

void BatteryMonitor_MainFunction(void)
{

	uint16_t measured_voltage_mv;

	if (ADC_Driver_ReadBatteryMv(&measured_voltage_mv) == true)
	{
		battery_voltage_mv = measured_voltage_mv;
	}

    switch (battery_state)
    {
        case BATTERY_STATE_NORMAL:

            if (battery_voltage_mv < BATTERY_CRITICAL_ENTER_MV)
            {
                battery_state = BATTERY_STATE_CRITICAL;
            }
            else if (battery_voltage_mv < BATTERY_LOW_ENTER_MV)
            {
                battery_state = BATTERY_STATE_LOW;
            }
            else if (battery_voltage_mv > BATTERY_OVER_VOLTAGE_ENTER_MV)
            {
                battery_state = BATTERY_STATE_OVER_VOLTAGE;
            }
            else
            {
                /* Remain normal. */
            }

            break;

        case BATTERY_STATE_LOW:

            if (battery_voltage_mv < BATTERY_CRITICAL_ENTER_MV)
            {
                battery_state = BATTERY_STATE_CRITICAL;
            }
            else if (battery_voltage_mv >= BATTERY_LOW_EXIT_MV)
            {
                battery_state = BATTERY_STATE_NORMAL;
            }
            else
            {
                /* Remain in low-voltage state. */
            }

            break;

        case BATTERY_STATE_CRITICAL:

            if (battery_voltage_mv >= BATTERY_CRITICAL_EXIT_MV)
            {
                if (battery_voltage_mv < BATTERY_LOW_EXIT_MV)
                {
                    battery_state = BATTERY_STATE_LOW;
                }
                else
                {
                    battery_state = BATTERY_STATE_NORMAL;
                }
            }
            else
            {
                /* Remain critical. */
            }

            break;

        case BATTERY_STATE_OVER_VOLTAGE:

            if (battery_voltage_mv <= BATTERY_OVER_VOLTAGE_EXIT_MV)
            {
                battery_state = BATTERY_STATE_NORMAL;
            }
            else
            {
                /* Remain in over-voltage state. */
            }

            break;

        default:

            battery_state = BATTERY_STATE_NORMAL;
            break;
    }
}

uint16_t BatteryMonitor_GetVoltageMv(void)
{
    return battery_voltage_mv;
}

battery_state_t BatteryMonitor_GetState(void)
{
    return battery_state;
}

bool BatteryMonitor_IsLowVoltage(void)
{
    return ((battery_state == BATTERY_STATE_LOW) ||
            (battery_state == BATTERY_STATE_CRITICAL));
}
