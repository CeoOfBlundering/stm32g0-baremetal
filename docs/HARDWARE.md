# Hardware

Fill in as wiring is decided. If something is not written here, it is not decided yet.

## Boards
| Board | Role | Logic level | Notes |
|---|---|---|---|
| Nucleo-G071RB | Main MCU | 3.3 V | ST-LINK virtual COM port on USART2 (PA2 TX / PA3 RX, AF1; UM2324 §6.9) |
| Arduino Due R3 (SAM3X8E, Cortex-M3, 84 MHz) | Second node: I2C slave, CAN node 2, TMR diversity | **3.3 V** | I/O pins are NOT 5 V tolerant: never feed them 5 V signals. Has 2 on-chip CAN controllers (CAN0/CAN1) → can use the spare TJA1050 boards directly, with RX level-shifted |
| Raspberry Pi 2 | Host tools / logging prototypes | 3.3 V | |

## Modules
| Module | Bus | Supply | Notes |
|---|---|---|---|
| MPU6050 (GY-521) | I2C, addr 0x68 | 3.3–5 V | WHO_AM_I (0x75) = 0x68. Check whether board has on-board pull-ups |
| MicroSD module | SPI | TODO | |
| GPS (NEO-6M or similar) | UART, 9600 baud NMEA | TODO | Header pins need soldering |
| MCP2515 + TJA1050 (x2) | SPI | **5 V** | MISO (SO) outputs 5 V → only to 5 V-tolerant (FT) STM32 pins or via level shifter; never directly to the Due. J1 = 120 Ω termination, fit on both ends of the bus |
| TJA1050 transceiver (x2, spare) | — | 5 V | No CAN controller. Usable with the Due's on-chip CAN; its RXD outputs 5 V → level-shift or divide before the Due |

## Pin map (STM32G071RB)
| Function | Pin | AF | Connected to | 5 V tolerant? |
|---|---|---|---|---|
| USART2 TX | PA2 | AF1 | ST-LINK VCP | FT_a: 5 V tolerant except in analog mode |
| USART2 RX | PA3 | AF1 | ST-LINK VCP | FT_a: 5 V tolerant except in analog mode |
| I2C SCL | TODO | | MPU6050 | |
| I2C SDA | TODO | | MPU6050 | |
| SPI SCK/MISO/MOSI/CS | TODO | | SD / MCP2515 | |

## Silicon revision
- REV_ID (DBGMCU_IDCODE[31:16]): TODO — read it once over the debugger; ES0418 errata apply per revision.

## Clock
- SYSCLK: TODO (HSI16 default = 16 MHz; PLL to 64 MHz if configured)
- APB: TODO

## Bench equipment
- 8-channel USB logic analyzer
- Multimeter
