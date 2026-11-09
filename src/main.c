#include "stm32g0xx.h"
#include "uart.h"

int main(void)
{
    uart_init();
    for (;;) {
        uart_write_string("hello\r\n");
        for (volatile uint32_t i = 0; i < 200000U; i++) { }
    }
}

