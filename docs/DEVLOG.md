# Devlog

Short entries, newest at the top. What was done, what broke, what is next.

---

## 2026-11-19
- Ring buffers: added `__COMPILER_BARRIER()` (CMSIS, `cmsis_gcc.h`) between the data access
  and the index update on the main-context side: `uart_write_byte()` (write `tx_buf`, then
  publish `tx_head`) and `uart_read_byte()` (read `rx_buf`, then advance `rx_tail`).
  `volatile` only orders the volatile accesses; the plain `rx_buf`/`tx_buf` accesses may be
  moved across them, letting the ISR see a slot before it is written or after it is freed.
- No barrier on the ISR side: `main` cannot preempt `USART2_IRQHandler`, so it never sees the
  ISR's two steps half-done (an RX-side barrier was tried and removed). On TX, `tx_buf` must
  be read before the volatile `TDR` store anyway. Revisit if another ISR ever touches the
  buffers (calling the UART API from an ISR also breaks SPSC and can deadlock on a full TX
  buffer).
- Verified: echo, and the 92-character string arrives complete and in order. "echo ready"
  twice after flashing is the debugger reset (one line per RESET button press).
- Next: line editor for the CLI.

---

## 2026-11-14
- USART2 RX, first polling (PA3/AF1, RE) with an echo test, then interrupt-driven:
  RXNEIE + `NVIC_EnableIRQ(USART2_IRQn)`, `USART2_IRQHandler` pushes into a 64-byte SPSC
  ring buffer, `uart_read_byte()` is now non-blocking. 920 B flash.
- Overrun seen live: halting the core while typing sets ORE; only the first byte survives
  in RDR and, until ORE is cleared via `ICR.ORECF`, the receiver accepts nothing more.
  Polling version now clears ORE while waiting; the ISR clears it on entry.
- Gotchas: the CLion SVD view reads RDR when USART2 is expanded, which consumes the byte
  and clears RXNE (keep USART2 collapsed when testing RX). A debugger halt also stops the
  ISR, so "pause and type" is not a test of interrupts; a busy loop in `main` is. Inverted
  `while` on RXNE froze the read on first byte. Ring-buffer "full" must test
  `(head + 1) & MASK == tail`, not `head != tail` (which drops everything when empty).
- Verified: echo with a ~1 s busy loop in `main`; text typed during the loop arrives
  complete afterwards.
- USART2 TX moved to interrupts: `uart_write_byte()` queues into a 64-byte TX ring buffer
  and returns; the ISR feeds TDR on TXE and turns TXEIE off when the buffer is empty.
  First attempt put the TX code inside the RX branch and made the ISR the *producer*
  (pushing received bytes into the TX buffer); in TX the ISR is the consumer and only
  touches `tx_tail`.
- Verified: a 92-character string (longer than the 63-byte capacity) arrives complete and in
  order, so the buffer wraps and the full-buffer wait works; echo still works with both
  directions interrupt-driven.
- Next: line editor for the CLI (backspace, Enter, command dispatch).

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
