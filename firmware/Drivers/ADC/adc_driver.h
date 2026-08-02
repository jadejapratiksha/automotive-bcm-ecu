#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * ADC logical channels used by the BCM.
 */
typedef enum
{
    ADC_CH_BATTERY_VOLTAGE = 0,

    ADC_CH_MAX
} adc_channel_t;

void ADC_Driver_Init(void);

/*
 * Read raw ADC value.
 *
 * STM32F407 ADC is 12-bit:
 * 0 ... 4095
 */
bool ADC_Driver_ReadRaw(adc_channel_t channel,
                        uint16_t *raw_value);

/*
 * Read estimated battery voltage in millivolts.
 */
bool ADC_Driver_ReadBatteryMv(uint16_t *battery_mv);
#ifdef RENODE_SIMULATION

/*
 * Set the battery voltage returned by the ADC driver during
 * Renode simulation.
 */
void ADC_Driver_SetSimulatedBatteryMv(uint16_t battery_mv);

#endif

#endif /* ADC_DRIVER_H */
