#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

void RTOS_VehicleTask(void *argument);
void RTOS_LightingTask(void *argument);
void RTOS_BatteryTask(void *argument);
void RTOS_FaultTask(void *argument);
void RTOS_DiagnosticTask(void *argument);
void RTOS_CANTask(void *argument);

#endif /* RTOS_TASKS_H */
