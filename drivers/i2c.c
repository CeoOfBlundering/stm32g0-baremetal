//
// Created by Pars on 24.11.2026.
//

#include "i2c.h"
#include <stdbool.h>
#include "stm32g0xx.h"

#define GPIO_MODE_AF    2U   // MODER = 10b: alternate function (RM0444 §7.5.1)
#define GPIO_AF6_I2C1   6U // // DS12232: PB8/PB9 AF6 = I2C1_SCL/SDA

#define I2C_TIMING_PRESC 0x3U   // RM0444 Table 173: 16MHz, 100 kHz
#define I2C_TIMING_SCLL 0x13U
#define I2C_TIMING_SCLH 0xFU
#define I2C_TIMING_SDADEL 0x2U
#define I2C_TIMING_SCLDEL 0x4U  // End of Table 173

void i2c_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    (void)RCC->IOPENR; // read back: clock active 2 cycles after enable (RM0444 §5.2.17).
    RCC->APBENR1 |= RCC_APBENR1_I2C1EN;

    GPIOB->OTYPER |= GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9;

    GPIOB->AFR[1] &= ~((GPIO_AFRH_AFSEL8_Msk) | (GPIO_AFRH_AFSEL9_Msk));
    GPIOB->AFR[1] |= (GPIO_AF6_I2C1 << GPIO_AFRH_AFSEL8_Pos) | (GPIO_AF6_I2C1 << GPIO_AFRH_AFSEL9_Pos);

    GPIOB->MODER &= ~((GPIO_MODER_MODE8_Msk) | (GPIO_MODER_MODE9_Msk));
    GPIOB->MODER |= (GPIO_MODE_AF << GPIO_MODER_MODE8_Pos) | (GPIO_MODE_AF << GPIO_MODER_MODE9_Pos);

    I2C1->TIMINGR = (I2C_TIMING_PRESC << I2C_TIMINGR_PRESC_Pos)
                  | (I2C_TIMING_SCLL << I2C_TIMINGR_SCLL_Pos)
                  | (I2C_TIMING_SCLH << I2C_TIMINGR_SCLH_Pos)
                  | (I2C_TIMING_SDADEL << I2C_TIMINGR_SDADEL_Pos)
                  | (I2C_TIMING_SCLDEL << I2C_TIMINGR_SCLDEL_Pos);
    I2C1->CR1 |= I2C_CR1_PE;
}