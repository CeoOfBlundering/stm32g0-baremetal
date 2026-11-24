#include "stm32g0xx.h"
#include "uart.h"
#include "cli.h"
#include "commands.h"
#include "i2c.h"

static struct line_editor le;
static void echo_uart(char c) {uart_write_byte((uint8_t)c);}

int main(void) {
    uart_init();
    i2c_init();
    cli_init(echo_uart);

    le_init(&le, echo_uart);
    uart_write_string("> ");
    for (;;) {
        uint8_t c;
        while (uart_read_byte(&c)) {
            if (le_feed(&le, (char)c)) {
                char *argv[CLI_MAX_ARGS];
                bool truncated;
                size_t argc = cli_tokenize(le.line, argv, CLI_MAX_ARGS, &truncated);
                cli_dispatch(cli_commands, cli_commands_count, argc, argv, truncated);
                uart_write_string("> ");
            }
        }
    }
}