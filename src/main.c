#include "stm32g0xx.h"
#include "uart.h"
#include "cli.h"

static le_t le;
static void echo_uart(char c) {uart_write_byte((uint8_t)c);}

int main(void)
{
    uart_init();
    le_init(&le, echo_uart);
    uart_write_string("> ");
    for (;;) {
        uint8_t c;
        while (uart_read_byte(&c)) {
            if (le_feed(&le, (char)c)) {
                uart_write_string("got: ");
                uart_write_string(le.line);
                uart_write_string("\r\n> ");
            }
        }
    }
}