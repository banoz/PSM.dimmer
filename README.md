# PSM Dimmer Firmware (CH32V003J4M6)

This firmware targets **CH32V003J4M6** and implements the requested event-driven behavior:

- `PC4` is configured as ADC input.
- `PC2` is configured as push/pull output.
- `PC1` is configured as interrupt input on falling edge.

Runtime flow:

1. Falling-edge interrupt on `PC1`:
   - Supplies latest mapped ADC value to PSM flow.
   - Evaluates skip decision in PSM logic.
   - Sets `PC2` output based on skip decision.
   - Starts a new ADC conversion.
   - Requests sleep.
2. ADC end-of-conversion interrupt:
   - Reads ADC value.
   - Applies reused PSM hysteresis/value update.
   - Maps ADC value to the working range.
   - Requests sleep.

Main loop sleeps with `WFI` and wakes only for interrupts.

## Files

- `src/main.c`: MCU/peripheral initialization, ISR flow, and sleep loop.
- `src/psm.h`: PSM constants and interfaces.
- `src/psm.c`: Reused PSM accumulator + hysteresis logic adapted from your Arduino/PlatformIO references.

## Dev Container

This repository now includes a devcontainer based on `islandc/wch-riscv-devcontainer`.

Host prerequisite:

- Set `WCH_SDK` on the host to your local `CH32V003EVT` folder before opening the container.

Bash example:

```bash
export WCH_SDK=/absolute/path/to/CH32V003EVT
```

The devcontainer mounts that path into the container as `/opt/wch-sdk` and sets `WCH_SDK=/opt/wch-sdk` inside the container.

Recommended workflow:

1. Install the VS Code Dev Containers extension.
2. Set `WCH_SDK` on the host.
3. Run `Dev Containers: Reopen in Container`.
4. Let the container finish setup.
5. Run the `Build CH32V003 firmware` task or configure/build with CMake inside the container.

The devcontainer runs `setup-devcontainer` after creation so the helper scripts and udev rule installer are copied into `.vscode/setup/`.

## CMake build

This repository now contains a CMake-based firmware build for **CH32V003J4M6**.

The build uses:

- GNU RISC-V embedded toolchain (`riscv-none-elf-gcc`)
- WCH CH32V003 EVT package (`CH32V00x_StdPeriph_Driver` + startup/system files)
- The linker script at `ld/ch32v003j4m6.ld`

### 1. Configure

Inside the devcontainer:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=/opt/gcc-riscv-none-elf/gcc-riscv-none-elf.cmake \
  -DWCH_SDK=/opt/wch-sdk
```

Expected folders under `WCH_SDK`:

- `CH32V00x_StdPeriph_Driver/inc/ch32v00x.h`
- `CH32V00x_StdPeriph_Driver/src/*.c`
- `EVT/EXAM/SRC/Startup/startup_ch32v00x.S`
- `EVT/EXAM/SRC/Startup/system_ch32v00x.c`

### 2. Build

```bash
cmake --build build
```

Output files:

- `build/psm_dimmer.elf`
- `build/psm_dimmer.hex`
- `build/psm_dimmer.bin`
- `build/psm_dimmer.map`

### 3. Clean

```bash
cmake --build build --target clean
```

### Optional overrides

- `-DWCH_SDK=<path>` to point at your EVT folder.
- `-DTARGET_NAME=<name>` to change the output firmware basename.

## Hardware mapping check

`main.c` uses `ADC_Channel_2` for `PC4`.

If your exact package/board routing differs, update:

- `ADC_CHANNEL_PC4` in `src/main.c`

based on your CH32V003J4M6 datasheet pin/channel table.
