# Devlog

Short entries, newest at the top. What was done, what broke, what is next.

---

## 2026-11-09
- Added reference extracts in `ref/` (git-ignored), ES0418 errata.
- Project skeleton: CMake + presets, toolchain file, linker script, CMSIS headers, startup.
  Empty `main` builds with `-Wall -Wextra -Werror`: 532 B flash. Vector table checked:
  SP = 0x20009000, reset handler in flash. Flashed and debugged from CLion via
  STM32CubeCLT 1.22 (ST-LINK GDB server, GCC 14.3): download verified, silicon rev Y.
- USART2 TX driver (polling) in `drivers/uart.c`: `uart_init`, `uart_write_byte`,
  `uart_write_string`. HSI16, BRR = 139 for 115200 baud (-0.08 % error). "hello" seen on the
  ST-LINK VCP (CLion Serial Port Monitor) and decoded with the logic analyzer in PulseView:
  68 65 6C 6C 6F 0D 0A, LSB first, ~86 us per byte, no idle gap between bytes.
- Gotchas: CLion created `uart.cpp` (project is C only, so it was silently skipped →
  undefined reference at link). Arduino D1 on this Nucleo is PC4, not PA2; PA2 is only on
  the ST morpho header. Cheap logic analyzer fails through a USB hub.
- Next: interrupt-driven TX with a ring buffer, then RX (PA3) for the CLI.

## 2026-11-06
- Set up AGENTS.md / CLAUDE.md and docs structure.
- Next: USART2 TX driver (polling), verify on ST-LINK VCP and logic analyzer.
