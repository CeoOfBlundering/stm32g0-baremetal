//
// Created by Pars on 9.11.2026.
//

#ifndef STM32_BAREMETAL_UART_H
#define STM32_BAREMETAL_UART_H

#include <stdint.h>

void uart_init(void);
void uart_write_byte(uint8_t byte);
void uart_write_string(const char *str);
uint8_t uart_read_byte(void);
#endif //STM32_BAREMETAL_UART_H
