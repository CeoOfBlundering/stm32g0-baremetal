//
// Created by Pars on 9.11.2026.
//

#include "uart.h"
#include "stm32g0xx.h"
#define GPIO_MODE_AF 0b10U
#define UART_CLK_HZ   16000000U   // HSI16, system default at boot.
#define UART_BAUD     115200U

#define RX_BUF_SIZE 64U
#define RX_BUF_MASK (RX_BUF_SIZE - 1U)

static uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static volatile uint32_t rx_dropped;   // Bytes lost while buffer is full

#define TX_BUF_SIZE 64U
#define TX_BUF_MASK (TX_BUF_SIZE - 1U)

static uint8_t tx_buf[TX_BUF_SIZE];
static volatile uint8_t tx_head;
static volatile uint8_t tx_tail;

void uart_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    GPIOA->MODER &= ~GPIO_MODER_MODE2_Msk;
    GPIOA->MODER |= (GPIO_MODE_AF << GPIO_MODER_MODE2_Pos);

    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk);
    GPIOA->AFR[0] |= (1U << GPIO_AFRL_AFSEL2_Pos);

    GPIOA->MODER &= ~GPIO_MODER_MODE3_Msk;
    GPIOA->MODER |= (GPIO_MODE_AF << GPIO_MODER_MODE3_Pos);

    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL3_Msk);
    GPIOA->AFR[0] |= (1U << GPIO_AFRL_AFSEL3_Pos);

    USART2->BRR = (UART_CLK_HZ + UART_BAUD / 2U) / UART_BAUD;   // = 139, RM0444 §33.5.7
    USART2->CR1 |= USART_CR1_TE | USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE; // Enable USART, RX, TX, Interrupt
    NVIC_EnableIRQ(USART2_IRQn);
}

void uart_write_byte(uint8_t byte) {
    uint8_t next = (uint8_t)((tx_head + 1U) & TX_BUF_MASK);
    while (next == tx_tail){}
    tx_buf[tx_head] = byte;
    __COMPILER_BARRIER();
    tx_head = next;
    ATOMIC_SET_BIT(USART2->CR1, USART_CR1_TXEIE_TXFNFIE);
}

void uart_write_string(const char *str) {
    while (*str != '\0') {
        uart_write_byte((uint8_t)*str);
        str++;
    }
}

bool uart_read_byte(uint8_t *out) {
    if (rx_head == rx_tail) {
        return false; // Buffer empty, nothing to read
    }
    *out = rx_buf[rx_tail];
    __COMPILER_BARRIER();
    rx_tail = (uint8_t)((rx_tail + 1U) & RX_BUF_MASK);
    return true;
}

uint32_t uart_get_rx_dropped(void) {
    return rx_dropped;
}

void USART2_IRQHandler(void) {
    if ((USART2->ISR & USART_ISR_ORE) != 0U) {
        USART2->ICR = USART_ICR_ORECF;
    }
    if ((USART2->ISR & USART_ISR_RXNE_RXFNE) != 0U) {
        uint8_t byte = (uint8_t)USART2->RDR;
        uint8_t next = (uint8_t)((rx_head + 1U) & RX_BUF_MASK);
        if (next != rx_tail) {
            rx_buf[rx_head] = byte;
            rx_head = next;
        }
        else {
            rx_dropped++;
        }
    }
    if (((USART2->CR1 & USART_CR1_TXEIE_TXFNFIE) != 0U) &&
    ((USART2->ISR & USART_ISR_TXE_TXFNF) != 0U)) {
        if (tx_head != tx_tail) {
            USART2->TDR = tx_buf[tx_tail];
            tx_tail = (uint8_t)((tx_tail + 1U) & TX_BUF_MASK);
        } else {
            USART2->CR1 &= ~USART_CR1_TXEIE_TXFNFIE;
        }
    }
}