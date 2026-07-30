#include "App/DoorManager/door_manager.h"
#include "Drivers/GPIO/gpio_driver.h"

void DoorManager_Init(void)
{
    /*
     * No additional initialization is required currently.
     * The physical GPIO is initialized by GPIO_Driver_Init().
     */
}

door_state_t DoorManager_GetFrontLeftState(void)
{
    gpio_state_t gpio_state;

    gpio_state = GPIO_Driver_Read(GPIO_CH_DOOR_FL);

    /*
     * The door switch uses active-low logic:
     *
     * GPIO LOW  = door open
     * GPIO HIGH = door closed
     */
    if (gpio_state == GPIO_STATE_LOW)
    {
        return DOOR_STATE_OPEN;
    }

    return DOOR_STATE_CLOSED;
}
