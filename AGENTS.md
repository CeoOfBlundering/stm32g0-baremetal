# AGENTS.md

Bare-metal firmware for the STM32G071RB (Nucleo-G071RB), written by a student to learn
embedded systems from first principles. Read this file fully before doing anything.

## Project facts
- MCU: STM32G071RB, Cortex-M0+, 64 MHz max. Reference manual: RM0444. Datasheet: DS12232.
- Toolchain: arm-none-eabi-gcc, CMake, CLion. Flash/debug via the on-board ST-LINK.
- No HAL, no LL, no CubeMX-generated code. Registers are accessed through the CMSIS device
  header (`stm32g071xx.h`) only.
- Hardware details, pinout and voltage levels: `docs/HARDWARE.md`. Never guess a pin, an
  alternate-function number or a clock value: look it up there or in RM0444, and ask if
  it is not documented.

## Roles: who writes what
The owner must be able to explain every line in an interview. Follow these rules:
- **First driver for each protocol** (UART, I2C, SPI, CAN, DMA, timers): the owner writes it.
  Review it, point to the RM0444 section, explain bugs, suggest fixes as small diffs.
  Do not write it unless explicitly asked.
- **After that, you may write any code, including drivers.** For every change that
  touches peripheral registers, explain in your reply which registers/bits you used,
  why, and the RM0444 section. Keep that explanation out of the code (see Comments).
- If the owner asks "why" about any line, answer with the mechanism (registers, timing,
  bus behaviour), not just the fix.
- Never commit on the owner's behalf; the owner reviews and commits.

## Reference docs
- Local only (`ref/`, git-ignored). Start with `ref/README.md`: it maps each file to its
  RM0444 / DS12232 / UM2324 pages.
- For register and bit names, search the CMSIS header `stm32g071xx.h` first.
- For behaviour and configuration sequences, read only the relevant chapter extract
  (e.g. `ref/rm0444_ch33_usart.md`). Figures do not survive extraction; if a table looks
  garbled, ask the owner to check the PDF page.
- RM0444 covers the whole G0x1 family. Before using a peripheral, confirm the G071 has it
  (DS12232). The G071 has no FDCAN.
- Check `ref/es0418_errata.md` for the peripheral in question before blaming the code
  for strange behaviour. Errata depend on silicon revision (REV_ID in DBGMCU_IDCODE;
  see `docs/HARDWARE.md`).

## Coding standard
- C11. Fixed-width types (`uint32_t` etc.). No dynamic allocation (`malloc`) in firmware.
- Aim for MISRA C:2012 style: no implicit conversions between signed/unsigned, every
  `switch` has `default`, no recursion, one exit point is preferred but not mandatory.
- `volatile` for anything shared with an ISR. Keep ISRs short; set flags or push to a
  ring buffer, do the work in the main loop.
- Register writes: read-modify-write with named CMSIS masks
  (`USART_CR1_UE`), never magic numbers. If a number is unavoidable, it is a named
  `#define` with its derivation.
- Naming: `module_verb_noun()` (e.g. `uart_write_byte`), `snake_case`, macros `UPPER_CASE`.
- Every driver: `drivers/<name>.h` (public API) + `drivers/<name>.c`. Static for anything private.

## Comments
- Do not comment *what* the code does; the code says that.
- Do comment *why*: a non-obvious choice, a hardware quirk, a timing constraint,
  and the RM0444 section it relies on. Example:
  `// BRR = fCK / baud = 64 MHz / 115200 ≈ 556 (RM0444 §33.5.7)`
- Longer explanations go in `docs/`, not in the code.

## Changes and history
- Commit messages: imperative summary line (≤ 72 chars), blank line, then **why** the
  change was made and what was verified (e.g. "checked TX waveform on logic analyzer").
- Architectural or hard-to-reverse decisions (interrupt vs polling, buffer sizes,
  clock tree, library choices) get a short entry in `docs/DECISIONS.md`.
- At the end of a work session, add a few lines to `docs/DEVLOG.md`: what was done,
  what broke, what is next. Keep entries short.
- Do not create new .md files without asking.

## Verification
- Code must build with `-Wall -Wextra -Werror` before you call a task done.
- Say clearly what you could not verify (you cannot see the board or the scope).
  Hardware behaviour is confirmed by the owner, not assumed.
