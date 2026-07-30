#ifndef DOOR_MANAGER_H
#define DOOR_MANAGER_H

/*
 * Logical door states used by the BCM application.
 */
typedef enum
{
    DOOR_STATE_CLOSED = 0,
    DOOR_STATE_OPEN
} door_state_t;

/*
 * Initialize the Door Manager module.
 */
void DoorManager_Init(void);

/*
 * Read the current front-left door state.
 */
door_state_t DoorManager_GetFrontLeftState(void);

#endif /* DOOR_MANAGER_H */