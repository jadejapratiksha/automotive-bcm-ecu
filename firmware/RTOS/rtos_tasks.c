#include "RTOS/rtos_tasks.h"

#include "cmsis_os2.h"
#include "Simulation/sim_interface.h"

#include "App/VehicleStateManager/vehicle_state_manager.h"
#include "App/LightingManager/lighting_manager.h"
#include "App/BatteryMonitor/battery_monitor.h"

#include "Services/FaultManager/fault_manager.h"
#include "Services/DiagnosticManager/diagnostic_manager.h"
#include "Services/CANService/can_service.h"


void RTOS_VehicleTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        VehicleStateManager_MainFunction();
#ifdef RENODE_SIMULATION
        SimInterface_MainFunction();
#endif

        osDelay(50U);
    }
}


void RTOS_LightingTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        LightingManager_MainFunction();

        osDelay(20U);
    }
}


void RTOS_BatteryTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        BatteryMonitor_MainFunction();

        osDelay(100U);
    }
}


void RTOS_FaultTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        FaultManager_MainFunction();

        osDelay(100U);
    }
}


void RTOS_DiagnosticTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        DiagnosticManager_MainFunction();

        osDelay(100U);
    }
}


void RTOS_CANTask(void *argument)
{
    (void)argument;

    /*
        * Initialize CAN after the scheduler and CAN RX queue
        * have been created.
        */
       CAN_Driver_Init();
       CANService_Init();
    for (;;)
    {
        CANService_MainFunction();

        osDelay(10U);
    }
}
