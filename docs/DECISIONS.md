# Decisions

One entry per decision that is hard to reverse or worth explaining in an interview.
Newest at the top. Keep each entry under ~10 lines.

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
