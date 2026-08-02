#include "Simulation/sim_interface.h"

#include <string.h>

#include "Simulation/sim_commands.h"
#include "Simulation/sim_transport_uart.h"


#define SIM_COMMAND_BUFFER_SIZE    (64U)


static char sim_command_buffer[SIM_COMMAND_BUFFER_SIZE];

static uint16_t sim_command_index = 0U;

static volatile uint8_t sim_command_ready = 0U;


void SimInterface_Init(void)
{
    memset(
        sim_command_buffer,
        0,
        sizeof(sim_command_buffer)
    );

    sim_command_index = 0U;
    sim_command_ready = 0U;

    SimTransportUart_Init();
}


void SimInterface_ProcessByte(uint8_t received_byte)
{
    if ((received_byte == '\r') ||
        (received_byte == '\n'))
    {
        if (sim_command_index > 0U)
        {
            sim_command_buffer[sim_command_index] = '\0';
            sim_command_ready = 1U;
        }

        return;
    }

    /*
     * Ignore input only while a completed command is waiting.
     */
    if (sim_command_ready != 0U)
    {
        return;
    }

    if (sim_command_index < (SIM_COMMAND_BUFFER_SIZE - 1U))
    {
        sim_command_buffer[sim_command_index] =
            (char)received_byte;

        sim_command_index++;
    }
    else
    {
        sim_command_index = 0U;

        memset(
            sim_command_buffer,
            0,
            sizeof(sim_command_buffer)
        );

        SimTransportUart_SendLine(
            "ERROR COMMAND TOO LONG"
        );
    }
}

void SimInterface_MainFunction(void)
{
    if (sim_command_ready == 0U)
    {
        return;
    }

    SimCommands_Process(sim_command_buffer);

    sim_command_ready = 0U;
    sim_command_index = 0U;

    memset(
        sim_command_buffer,
        0,
        sizeof(sim_command_buffer)
    );
}
