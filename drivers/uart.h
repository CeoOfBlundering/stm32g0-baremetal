//
// Created by Pars on 9.11.2026.
//

#ifndef STM32_BAREMETAL_UART_H
#define STM32_BAREMETAL_UART_H

#include <stdint.h>
#include <stdbool.h>

void uart_init(void);
void uart_write_byte(uint8_t byte);
void uart_write_string(const char *str);
bool uart_read_byte(uint8_t *out);
#endif //STM32_BAREMETAL_UART_H
