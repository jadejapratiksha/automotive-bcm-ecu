#ifndef VEHICLE_STATE_MANAGER_H
#define VEHICLE_STATE_MANAGER_H

#include <stdbool.h>

/*
 * Vehicle operating states.
 */
typedef enum
{
    VEHICLE_STATE_OFF = 0,
    VEHICLE_STATE_ACCESSORY,
    VEHICLE_STATE_RUNNING,

    VEHICLE_STATE_MAX
} vehicle_state_t;

/*
 * Initialize the Vehicle State Manager.
 */
void VehicleStateManager_Init(void);

/*
 * Periodic processing function.
 */
void VehicleStateManager_MainFunction(void);

/*
 * Request a new vehicle state.
 *
 * Returns:
 * true  = transition accepted
 * false = transition rejected
 */
bool VehicleStateManager_SetState(vehicle_state_t requested_state);

/*
 * Return the current vehicle state.
 */
vehicle_state_t VehicleStateManager_GetState(void);

/*
 * Return true when the vehicle is running.
 */
bool VehicleStateManager_IsRunning(void);

#endif /* VEHICLE_STATE_MANAGER_H */