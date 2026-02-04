# FreeRTOS RPi2 Bare-Metal

Bare-metal FreeRTOS port for Raspberry Pi 2B v1.2 (BCM2837 SoC, Cortex-A53 in AArch32 mode).

## Quick Start

```bash
# Build
./build_rpi2.sh

# Output: Build/kernel7.img (~36KB)
```

## SD Card Setup

1. Format FAT32
2. Copy from [RPi firmware](https://github.com/raspberrypi/firmware/tree/master/boot):
   - `bootcode.bin`, `start.elf`, `fixup.dat`
3. Copy `Build/kernel7.img`
4. Optional `config.txt`:
   ```
   kernel=kernel7.img
   enable_uart=1
   ```

## Project Structure

```
Source/              # Application code
├── main.c           # Main application & tasks
├── uart.c/h         # UART driver (uart_printf)
├── rpi2_support.c   # Hardware support
└── FreeRTOSConfig.h # FreeRTOS config

FreeRTOS/            # FreeRTOS kernel (submodule, unmodified)
├── portable/GCC/ARM_CA9/  # ARM port

Startup/
├── startup_rpi2.S   # Boot code, vector table
└── link_rpi2.ld     # Linker script (0x8000 boot)
```

## Hardware Status

**Tested and working on real RPi2 BCM2837**:
- HYP mode detection and exit
- Secondary CPU parking (CPU1-3)
- FreeRTOS scheduler running
- UART output via GPIO14/15 (115200 baud)
- 32MB heap allocation

## Boot Sequence

GPU firmware boots ARM in HYP mode. Startup code:
1. Parks secondary CPUs (CPU1-3) via WFI
2. Detects HYP mode → `eret` to SVC mode
3. Skips FPU init (not needed)
4. Sets up IRQ stack (8KB) and SVC stack (16KB)
5. Clears BSS → calls `main()`

Debug output: `XYIVHYEN123456789` = successful boot

## Memory Map

| Address | Description |
|---------|-------------|
| 0x00008000 | FreeRTOS kernel start |
| 0x3F000000 | Peripheral base |
| 0x3F201000 | UART0 (PL011) |
| 0x40000000 | ARM local peripherals |

## Debugging

Use `uart_printf()` from `Source/uart.c`:
```c
#include "uart.h"
uart_printf("Task started, priority=%d\n", priority);
```

If FreeRTOS internals need debugging:
```c
// In FreeRTOS/portable/GCC/ARM_CA9/port.c, add:
extern void uart_printf(const char *format, ...);
```

## Key Notes

- **kernel7.img**: Required filename for RPi firmware (ARMv7/ARMv8-32)
- **FPU disabled**: Skipped to avoid undefined instruction faults
- **HYP mode**: GPU boots in HYP, code drops to SVC for FreeRTOS
- **Heap**: 32MB (configurable in FreeRTOSConfig.h)

## Documentation

| File | Purpose |
|------|---------|
| `README.md` | Project overview |
| `DEVELOPMENT_GUIDE.md` | Debugging methodology |
| `BOOT_DEBUGGING.md` | Boot troubleshooting |
| `SD_CARD_SETUP.md` | SD card preparation |
| `SETUP_FREERTOS.md` | FreeRTOS integration |
| `CONTRIBUTING.md` | Contribution guidelines |

## Related

- seL4 FreeRTOS VM: `camkes-vm-examples/projects/vm-examples/apps/Arm/vm_freertos/`
- Research docs: `research-docs/archive/freertos-*`
