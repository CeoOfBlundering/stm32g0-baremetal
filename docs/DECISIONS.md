# Decisions

One entry per decision that is hard to reverse or worth explaining in an interview.
Newest at the top. Keep each entry under ~10 lines.

---

## D-004 — USART2 TX is interrupt-driven; writers block only when the buffer is full
- **Date:** 2026-11-14
- **Context:** Polling TX blocks `main` ~87 µs per byte (~9 ms for a 100-character CLI help
  text).
- **Decision:** 64-byte TX ring buffer. `main` is the producer (`tx_head`), the ISR the
  consumer (`tx_tail`). TXEIE is enabled only while there is data: TXE stays set whenever the
  transmitter is idle, so an always-on TXE interrupt would fire continuously. The writer sets
  TXEIE after queueing; the ISR clears it when the buffer is empty.
  Unlike RX, a full TX buffer makes `uart_write_byte()` wait instead of dropping, because
  we produce the data and losing half a message is worse than a short wait.
- **Race note:** CR1 is now written from both `main` and the ISR. `main` uses
  `ATOMIC_SET_BIT` (briefly masks interrupts on the M0+, which has no LDREX/STREX) so its
  read-modify-write cannot overwrite a change the ISR made in between. The ISR uses plain
  `&=`, since `main` cannot preempt it.
- **Consequences:** printing no longer stalls `main` unless more than 63 bytes are queued at
  once. Output order is preserved. DMA could replace the ISR later behind the same API.

---

## D-003 — USART2 RX is interrupt-driven with a lock-free ring buffer
- **Date:** 2026-11-14
- **Context:** Polling RX loses bytes whenever `main` is busy for more than one frame
  (~87 µs at 115200 baud): the second byte overruns RDR and ORE blocks the receiver.
  The CLI and later the GPS parser need every byte while `main` does other work.
- **Options considered:** polling; polling with the 8-byte RX FIFO (`FIFOEN`, delays the
  problem to ~0.7 ms); RXNE interrupt + ring buffer; DMA into a circular buffer.
- **Decision:** RXNE interrupt + 64-byte single-producer/single-consumer ring buffer.
  The ISR only writes `head`, the reader only writes `tail`; both are `volatile uint8_t`,
  so no lock is needed on the single-core M0+. Size is a power of two so wrap is `& 63`
  (the M0+ has no hardware divide). Full buffer drops the new byte and counts it in
  `rx_dropped`. ORE is cleared in the ISR to avoid an interrupt storm.
- **Consequences:** `uart_read_byte()` is non-blocking. ~87 µs × 63 bytes of slack before
  data loss. DMA can replace the ISR later without changing the API. FIFO stays disabled
  for now so RXNE/RDR behave one byte at a time.

---

## D-002 — CMSIS from upstream GitHub, CMake instead of a CubeIDE project
- **Date:** 2026-11-09
- **Context:** CubeIDE projects pull in HAL, `.ioc` and Eclipse metadata, which conflict with
  D-001 and with the CLion/CMake workflow.
- **Decision:** Device and core headers, startup file and `system_stm32g0xx.c` copied
  unmodified from `STMicroelectronics/cmsis_device_g0` v1.4.5 and `ARM-software/CMSIS_5`
  (versions in `third_party/README.md`). Linker script written by hand for 128 KB flash /
  36 KB RAM. Build with CMake presets and arm-none-eabi-gcc.
- **Consequences:** No code generation; clock, pins and peripherals are configured by hand.
  Vendor files are updated by copying a newer release.

---

## D-001 — No HAL, register-level drivers only
- **Date:** 2026-11
- **Context:** The goal of this repo is to understand peripherals at the register level.
- **Decision:** All drivers use CMSIS register definitions only; no HAL/LL/CubeMX.
- **Consequences:** Slower progress, more reading of RM0444; every driver must be tested
  on hardware. Portability to other STM32 families is not a goal.

<!-- Template:
## D-XXX — Title
- **Date:**
- **Context:** what problem forced a choice
- **Options considered:**
- **Decision:**
- **Consequences:** what gets easier, what gets harder
-->
