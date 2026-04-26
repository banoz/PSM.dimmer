# One-Shot Prompt For Coding Agent

Implement a complete CH32V003J4M6 firmware project in C with CMake, targeting the current devcontainer environment.

## Objective

Build firmware where:

1. PC4 is ADC input.
2. PC2 is push-pull output.
3. PC1 is falling-edge external interrupt input.
4. On each PC1 interrupt:
- Use the previously stored mapped ADC value (do not wait for conversion).
- Feed that mapped value into PSM skip logic.
- Set output on PC2 based on skip result.
- Trigger a new ADC conversion.
- Return quickly.
5. On ADC conversion complete interrupt:
- Read ADC value.
- Map ADC to working range.
- Store mapped value for use by the next PC1 interrupt.
6. MCU should be interrupt-driven and sleep between events.

## Behavior Requirements

1. Initial stored value must be 0.
2. First PC1 event must therefore skip activation and keep output low.
3. With ADC mapped value 0, logic must skip all interrupts and output must stay constantly low.
4. As ADC value increases, skip frequency must decrease and output high events must increase.
5. Logic must be deterministic and non-blocking in interrupts.

## Implementation Constraints

1. Use StdPeriph driver for CH32V003.
2. Keep ISR handlers minimal and fast.
3. No polling loops in normal runtime path.
4. Use WFI-based sleep strategy.
5. Use regular ADC conversion with EOC interrupt; start conversion from the PC1 EXTI handler.
6. Keep PSM logic isolated in its own module with clear API for:
- map raw ADC to logic/working value
- set stored value
- calculate skip decision

## Build And Environment Constraints

1. Use CMake only.
2. Assume devcontainer toolchain file at /opt/gcc-riscv-none-elf/gcc-riscv-none-elf.cmake.
3. Assume SDK root at /opt/wch-sdk with EVT startup and peripheral sources.
4. Ensure linker script and startup symbols are compatible with WCH startup, including _start entry and required highcode/global-pointer symbols.
5. Ensure post-build produces ELF, HEX, BIN, and size output.

## Deliverables

1. Complete source implementation.
2. Working CMake configuration.
3. Correct linker script integration.
4. Brief README section with build commands for configure/build/clean in the devcontainer.
5. Confirm successful build and provide resulting size summary.

## Acceptance Checks

1. Firmware builds without linker errors.
2. PC1 handler uses prior ADC result only and only starts a new conversion.
3. ADC complete handler updates stored mapped value for next cycle.
4. Value 0 produces constant output low.
5. Higher ADC produces fewer skips and more output high pulses.
