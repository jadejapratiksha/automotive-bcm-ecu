#ifndef LIGHTING_MANAGER_H
#define LIGHTING_MANAGER_H

/*
 * Initializes the Lighting Manager.
 */
void LightingManager_Init(void);

/*
 * Periodic function that controls the interior lamp
 * based on the current door state.
 */
void LightingManager_MainFunction(void);

#endif /* LIGHTING_MANAGER_H */