//
// Created by Pars on 22.11.2026.
//

#ifndef STM32_BAREMETAL_COMMANDS_H
#define STM32_BAREMETAL_COMMANDS_H

#include <stddef.h>

#include "cli.h"

// Command table for this firmware; pass both to cli_dispatch.
extern const struct cli_command cli_commands[];
extern const size_t cli_commands_count;

#endif //STM32_BAREMETAL_COMMANDS_H
