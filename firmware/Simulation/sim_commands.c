#include "Simulation/sim_commands.h"

#include <stdio.h>
#include <string.h>

#include "main.h"

#include "Simulation/sim_transport_uart.h"

#include "App/BatteryMonitor/battery_monitor.h"
#include "App/DoorManager/door_manager.h"
#include "App/VehicleStateManager/vehicle_state_manager.h"
#include "Services/FaultManager/fault_manager.h"
#include "Services/DiagnosticManager/diagnostic_manager.h"


#define SIM_STATUS_BUFFER_SIZE    (160U)
static char status_message[SIM_STATUS_BUFFER_SIZE];

static uint8_t SimCommands_GetInteriorLampState(void)
{
    GPIO_PinState lamp_pin_state;

    lamp_pin_state = HAL_GPIO_ReadPin(
        INTERIOR_LAMP_OUT_GPIO_Port,
        INTERIOR_LAMP_OUT_Pin
    );

    return (lamp_pin_state == GPIO_PIN_SET) ? 1U : 0U;
}


static void SimCommands_SendStatus(void)
{


    int written = snprintf(
        status_message,
        sizeof(status_message),
        "{"
        "\"vehicle_state\":%u,"
        "\"door_state\":%u,"
        "\"battery_mv\":%u,"
        "\"battery_state\":%u,"
        "\"interior_lamp\":%u,"
        "\"fault_mask\":%lu,"
        "\"dtc_mask\":%lu"
        "}",
        (unsigned int)VehicleStateManager_GetState(),
        (unsigned int)DoorManager_GetFrontLeftState(),
        (unsigned int)BatteryMonitor_GetVoltageMv(),
        (unsigned int)BatteryMonitor_GetState(),
        (unsigned int)SimCommands_GetInteriorLampState(),
        (unsigned long)FaultManager_GetActiveFaultMask(),
        (unsigned long)DiagnosticManager_GetDTCMask()
    );

    if ((written < 0) ||
        (written >= (int)sizeof(status_message)))
    {
        SimTransportUart_SendLine("ERROR STATUS BUFFER");
        return;
    }

    SimTransportUart_SendLine(status_message);
}


static void SimCommands_SendHelp(void)
{
    SimTransportUart_SendLine("SUPPORTED COMMANDS:");
    SimTransportUart_SendLine("PING");
    SimTransportUart_SendLine("GET STATUS");
    SimTransportUart_SendLine("HELP");
}


void SimCommands_Process(const char *command)
{
    if (command == NULL)
    {
        SimTransportUart_SendLine("ERROR NULL COMMAND");
        return;
    }

    if (strcmp(command, "PING") == 0)
    {
        SimTransportUart_SendLine("PONG");
    }
    else if (strcmp(command, "GET STATUS") == 0)
    {
        SimCommands_SendStatus();
    }
    else if (strcmp(command, "HELP") == 0)
    {
        SimCommands_SendHelp();
    }
    else
    {
        SimTransportUart_SendLine("ERROR UNKNOWN COMMAND");
    }
}
