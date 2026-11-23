//
// Created by Pars on 22.11.2026.
//

#include "commands.h"

#include <string.h>

#include "uart.h"

#define FW_VERSION "stm32-baremetal 0.1.0"

static void cmd_help(size_t argc, char *argv[]);
static void cmd_version(size_t argc, char *argv[]);
static void cmd_uart(size_t argc, char *argv[]);

const struct cli_command cli_commands[] = {
    { "help",    cmd_help,    "list commands" },
    { "version", cmd_version, "firmware version" },
    { "uart",    cmd_uart,    "uart stats: RX drop and overrun counters" },
};

const size_t cli_commands_count = sizeof(cli_commands) / sizeof(cli_commands[0]);

static void cmd_help(size_t argc, char *argv[]) {
    for (size_t i = 0U; i < cli_commands_count; i++) {
        cli_print(cli_commands[i].name);
        cli_print("  ");
        cli_print(cli_commands[i].help);
        cli_print("\r\n");
    }
    (void)argc;
    (void)argv;
}

static void cmd_version(size_t argc, char *argv[]) {
    cli_print(FW_VERSION "\r\n");
    (void)argc;
    (void)argv;
}

static void cmd_uart(size_t argc, char *argv[]) {
    if ((argc == 2U) && (strcmp(argv[1], "stats") == 0)) {
        cli_print("rx_dropped: ");
        cli_print_u32(uart_get_rx_dropped());
        cli_print("\r\nrx_overruns: ");
        cli_print_u32(uart_get_rx_overruns());
        cli_print("\r\n");
    }
    else {
        cli_print("usage: uart stats\r\n");
    }
}
