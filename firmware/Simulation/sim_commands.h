#ifndef SIM_COMMANDS_H
#define SIM_COMMANDS_H

/*
 * Process one complete null-terminated simulation command.
 *
 * Supported initially:
 *
 * PING
 * GET STATUS
 * HELP
 */
void SimCommands_Process(const char *command);

#endif /* SIM_COMMANDS_H */
