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
uint32_t uart_get_rx_dropped(void);    // bytes discarded: ring buffer full
uint32_t uart_get_rx_overruns(void);   // ORE events: ISR too late, >= 1 byte lost each
#endif //STM32_BAREMETAL_UART_H
