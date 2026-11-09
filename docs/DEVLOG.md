# Devlog

Short entries, newest at the top. What was done, what broke, what is next.

---

## 2026-11-09
- Added reference extracts in `ref/` (git-ignored), ES0418 errata.
- Project skeleton: CMake + presets, toolchain file, linker script, CMSIS headers, startup.
  Empty `main` builds with `-Wall -Wextra -Werror`: 532 B flash. Vector table checked:
  SP = 0x20009000, reset handler in flash. Flashed and debugged from CLion via
  STM32CubeCLT 1.22 (ST-LINK GDB server, GCC 14.3): download verified, silicon rev Y.
- Next: flash the empty build once, then USART2 TX driver (polling) on PA2/AF1.

## 2026-11-06
- Set up AGENTS.md / CLAUDE.md and docs structure.
- Next: USART2 TX driver (polling), verify on ST-LINK VCP and logic analyzer.
