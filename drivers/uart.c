//
// Created by Pars on 9.11.2026.
//

#include "uart.h"
#include "stm32g0xx.h"
#define GPIO_MODE_AF 0b10U
#define UART_CLK_HZ   16000000U   // HSI16, system default at boot.
#define UART_BAUD     115200U


void uart_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    GPIOA->MODER &= ~GPIO_MODER_MODE2_Msk;
    GPIOA->MODER |= (GPIO_MODE_AF << GPIO_MODER_MODE2_Pos);

    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk);
    GPIOA->AFR[0] |= (1U << GPIO_AFRL_AFSEL2_Pos);

    USART2->BRR = (UART_CLK_HZ + UART_BAUD / 2U) / UART_BAUD;   // = 139, RM0444 §33.5.7
    USART2->CR1 |= USART_CR1_TE | USART_CR1_UE;
}

void uart_write_byte(uint8_t byte) {
    while ((USART2->ISR & USART_ISR_TXE_TXFNF) == 0U) {
    }
    USART2->TDR = byte;
}

void uart_write_string(const char *str) {
    while (*str != '\0') {
        uart_write_byte((uint8_t)*str);
        str++;
    }
}