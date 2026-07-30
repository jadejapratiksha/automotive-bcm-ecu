#include "App/VehicleStateManager/vehicle_state_manager.h"

/*
 * Current state is private to this module.
 *
 * static prevents other source files from directly modifying it.
 */
static vehicle_state_t current_vehicle_state;

/*
 * Checks whether a requested transition is permitted.
 */
static bool VehicleStateManager_IsTransitionValid(
        vehicle_state_t current_state,
        vehicle_state_t requested_state)
{
    bool transition_valid = false;

    switch (current_state)
    {
        case VEHICLE_STATE_OFF:

            if ((requested_state == VEHICLE_STATE_OFF) ||
                (requested_state == VEHICLE_STATE_ACCESSORY))
            {
                transition_valid = true;
            }

            break;

        case VEHICLE_STATE_ACCESSORY:

            if ((requested_state == VEHICLE_STATE_OFF) ||
                (requested_state == VEHICLE_STATE_ACCESSORY) ||
                (requested_state == VEHICLE_STATE_RUNNING))
            {
                transition_valid = true;
            }

            break;

        case VEHICLE_STATE_RUNNING:

            if ((requested_state == VEHICLE_STATE_OFF) ||
                (requested_state == VEHICLE_STATE_ACCESSORY) ||
                (requested_state == VEHICLE_STATE_RUNNING))
            {
                transition_valid = true;
            }

            break;

        default:

            transition_valid = false;
            break;
    }

    return transition_valid;
}

void VehicleStateManager_Init(void)
{
    /*
     * Safe startup state.
     */
    current_vehicle_state = VEHICLE_STATE_OFF;
}

void VehicleStateManager_MainFunction(void)
{
    /*
     * No periodic processing is needed yet.
     *
     * Later, this function can process:
     * - ignition GPIO state
     * - CAN ignition commands
     * - sleep and wake requests
     * - invalid-state recovery
     */

    if (current_vehicle_state >= VEHICLE_STATE_MAX)
    {
        current_vehicle_state = VEHICLE_STATE_OFF;
    }
}

bool VehicleStateManager_SetState(vehicle_state_t requested_state)
{
    bool transition_valid;

    if (requested_state >= VEHICLE_STATE_MAX)
    {
        return false;
    }

    transition_valid =
        VehicleStateManager_IsTransitionValid(current_vehicle_state,
                                              requested_state);

    if (transition_valid == true)
    {
        current_vehicle_state = requested_state;
    }

    return transition_valid;
}

vehicle_state_t VehicleStateManager_GetState(void)
{
    return current_vehicle_state;
}

bool VehicleStateManager_IsRunning(void)
{
    return (current_vehicle_state == VEHICLE_STATE_RUNNING);
}