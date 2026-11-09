# Decisions

One entry per decision that is hard to reverse or worth explaining in an interview.
Newest at the top. Keep each entry under ~10 lines.

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
