#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "stm32f4xx_hal.h"

/*
 * GPIO Logical States
 */
typedef enum
{
    GPIO_STATE_LOW = 0,
    GPIO_STATE_HIGH
} gpio_state_t;

/*
 * GPIO Channels used by the BCM
 */
typedef enum
{
    GPIO_CH_DOOR_FL = 0,
    GPIO_CH_INTERIOR_LAMP,

    GPIO_CH_MAX
} gpio_channel_t;

/*
 * Initialize GPIO Driver
 */
void GPIO_Driver_Init(void);

/*
 * Read a GPIO channel
 */
gpio_state_t GPIO_Driver_Read(gpio_channel_t channel);

/*
 * Write a GPIO channel
 */
void GPIO_Driver_Write(gpio_channel_t channel,
                       gpio_state_t state);

#endif /* GPIO_DRIVER_H */