#include "Drivers/GPIO/gpio_driver.h"
#include "main.h"

void GPIO_Driver_Init(void)
{
    /*
     * CubeMX already initializes the GPIO hardware.
     * This function exists for future scalability.
     */
}

gpio_state_t GPIO_Driver_Read(gpio_channel_t channel)
{
    GPIO_PinState pin_state;

    switch(channel)
    {
        case GPIO_CH_DOOR_FL:

            pin_state = HAL_GPIO_ReadPin(
                            DOOR_FL_IN_GPIO_Port,
                            DOOR_FL_IN_Pin);

            return (pin_state == GPIO_PIN_SET)
                    ? GPIO_STATE_HIGH
                    : GPIO_STATE_LOW;

        default:

            return GPIO_STATE_LOW;
    }
}

void GPIO_Driver_Write(gpio_channel_t channel,
                       gpio_state_t state)
{
    switch(channel)
    {
        case GPIO_CH_INTERIOR_LAMP:

            HAL_GPIO_WritePin(
                    INTERIOR_LAMP_OUT_GPIO_Port,
                    INTERIOR_LAMP_OUT_Pin,
                    (state == GPIO_STATE_HIGH)
                    ? GPIO_PIN_SET
                    : GPIO_PIN_RESET);

            break;

        default:
            break;
    }
}
