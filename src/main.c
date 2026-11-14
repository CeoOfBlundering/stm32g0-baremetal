#include "stm32g0xx.h"
#include "uart.h"

int main(void)
{
    uart_init();
    uart_write_string("echo ready\r\n");
    for (;;) {
        uint8_t c;
        while (uart_read_byte(&c)) {
            uart_write_byte(c);
        }
    }
}