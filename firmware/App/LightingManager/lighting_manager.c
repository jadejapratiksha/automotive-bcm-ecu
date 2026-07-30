#include "App/LightingManager/lighting_manager.h"
#include "App/DoorManager/door_manager.h"
#include "Drivers/GPIO/gpio_driver.h"
#include "App/VehicleStateManager/vehicle_state_manager.h"
#include "App/BatteryMonitor/battery_monitor.h"

void LightingManager_Init(void)
{
    /*
     * Ensure the interior lamp starts in the OFF state.
     */
    GPIO_Driver_Write(GPIO_CH_INTERIOR_LAMP,
                      GPIO_STATE_LOW);
}

void LightingManager_MainFunction(void)
{
	 door_state_t door_state;
	    vehicle_state_t vehicle_state;
	    bool low_voltage_active;
	    bool vehicle_state_valid;

	    door_state = DoorManager_GetFrontLeftState();
	    vehicle_state = VehicleStateManager_GetState();
	    low_voltage_active = BatteryMonitor_IsLowVoltage();

	    vehicle_state_valid =
	        (vehicle_state < VEHICLE_STATE_MAX);

	    if ((door_state == DOOR_STATE_OPEN) &&
	        (low_voltage_active == false) &&
	        (vehicle_state_valid == true))
	    {
	        GPIO_Driver_Write(GPIO_CH_INTERIOR_LAMP,
	                          GPIO_STATE_HIGH);
	    }
	    else
	    {
	        GPIO_Driver_Write(GPIO_CH_INTERIOR_LAMP,
	                          GPIO_STATE_LOW);
	    }

}
