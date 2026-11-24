//
// Created by Pars on 9.11.2026.
//

#include "uart.h"
#include "stm32g0xx.h"
#define GPIO_MODE_AF    2U   // MODER = 10b: alternate function (RM0444 §7.5.1)
#define GPIO_AF1_USART2 1U   // DS12232: PA2/PA3 AF1 = USART2_TX/RX
#define UART_CLK_HZ   16000000U   // HSI16, system default at boot.
#define UART_BAUD     115200U

#define RX_BUF_SIZE 64U
#define RX_BUF_MASK (RX_BUF_SIZE - 1U)

static uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static volatile uint32_t rx_dropped;    // Bytes discarded because the ring buffer was full
static volatile uint32_t rx_overruns;   // ORE events; each one means at least one lost byte

#define TX_BUF_SIZE 64U
#define TX_BUF_MASK (TX_BUF_SIZE - 1U)

static uint8_t tx_buf[TX_BUF_SIZE];
static volatile uint8_t tx_head;
static volatile uint8_t tx_tail;

void uart_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    volatile uint32_t dummy = RCC->IOPENR; // Force the CPU to wait until register is open
    (void)dummy; // Avoid unused variable compiler warning.
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    GPIOA->MODER &= ~GPIO_MODER_MODE2_Msk;
    GPIOA->MODER |= (GPIO_MODE_AF << GPIO_MODER_MODE2_Pos);

    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk);
    GPIOA->AFR[0] |= (GPIO_AF1_USART2 << GPIO_AFRL_AFSEL2_Pos);

    GPIOA->MODER &= ~GPIO_MODER_MODE3_Msk;
    GPIOA->MODER |= (GPIO_MODE_AF << GPIO_MODER_MODE3_Pos);

    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL3_Msk);
    GPIOA->AFR[0] |= (GPIO_AF1_USART2 << GPIO_AFRL_AFSEL3_Pos);

    USART2->BRR = (UART_CLK_HZ + UART_BAUD / 2U) / UART_BAUD;   // = 139, RM0444 §33.5.7
    USART2->CR1 |= USART_CR1_TE | USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE;
    NVIC_EnableIRQ(USART2_IRQn);
}

void uart_write_byte(uint8_t byte) {
    uint8_t next = (uint8_t)((tx_head + 1U) & TX_BUF_MASK);
    while (next == tx_tail) {
        // Buffer full: wait for the ISR to make room instead of dropping output (D-004).
    }
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
    bool ok = (rx_head != rx_tail);

    if (ok) {
        *out = rx_buf[rx_tail];
        __COMPILER_BARRIER();
        rx_tail = (uint8_t)((rx_tail + 1U) & RX_BUF_MASK);
    }
    return ok;
}

uint32_t uart_get_rx_dropped(void) {
    return rx_dropped;
}

uint32_t uart_get_rx_overruns(void) {
    return rx_overruns;
}

void USART2_IRQHandler(void) {
    uint32_t isr = USART2->ISR;   // read once; a flag set after this re-pends the IRQ

    if ((isr & USART_ISR_ORE) != 0U) {
        // Overrun (RM0444 §33.5.6): the shift register was overwritten, so at least one
        // byte is lost. RDR still holds the previous byte and is read below.
        USART2->ICR = USART_ICR_ORECF;
        rx_overruns++;
    }
    if ((isr & USART_ISR_RXNE_RXFNE) != 0U) {
        uint8_t byte = (uint8_t)USART2->RDR;
        uint8_t next = (uint8_t)((rx_head + 1U) & RX_BUF_MASK);
        if (next != rx_tail) {
            // no barrier needed: main cannot preempt this ISR (see DEVLOG 2026-11-19)
            rx_buf[rx_head] = byte;
            rx_head = next;
        }
        else {
            rx_dropped++;
        }
    }
    if (((USART2->CR1 & USART_CR1_TXEIE_TXFNFIE) != 0U) &&
        ((isr & USART_ISR_TXE_TXFNF) != 0U)) {
        if (tx_head != tx_tail) {
            USART2->TDR = tx_buf[tx_tail];
            tx_tail = (uint8_t)((tx_tail + 1U) & TX_BUF_MASK);
        }
        else {
            USART2->CR1 &= ~USART_CR1_TXEIE_TXFNFIE;
        }
    }
}
