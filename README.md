# FreeRTOS for Raspberry Pi 2B v1.2 (BCM2837)

Bare-metal FreeRTOS port for Raspberry Pi 2B version 1.2 with BCM2837 SoC (Cortex-A53 @ 900MHz).

## Hardware

- **Board**: Raspberry Pi 2 Model B v1.2
- **SoC**: Broadcom BCM2837 (same as RPi3, clocked at 900MHz)
- **CPU**: Quad-core ARM Cortex-A53 (ARMv8, running in AArch32 mode)
- **RAM**: 1GB

## Project Structure

```
freertos_rpi2_bcm2837/
├── Source/                  # Application source code
│   ├── main.c              # Main application
│   ├── rpi2_support.c      # Hardware support functions
│   └── FreeRTOSConfig.h    # FreeRTOS configuration for BCM2837
├── FreeRTOS/               # FreeRTOS kernel (self-contained)
│   ├── include/            # FreeRTOS headers
│   ├── portable/           # Port-specific code (ARM_CA9)
│   └── *.c                 # FreeRTOS core source
├── Startup/
│   ├── startup_rpi2.S      # Boot code and vector table
│   └── link_rpi2.ld        # Linker script (boots at 0x8000)
├── Build/                  # Build output directory
│   └── kernel7.img         # Bootable image (generated)
├── build_rpi2.sh           # Build script
├── .gitignore             # Git ignore rules
└── README.md              # This file
```

## Dependencies

- ARM cross-compiler: `arm-none-eabi-gcc`
- Raspberry Pi firmware files (bootcode.bin, start.elf, fixup.dat)

**Note**: FreeRTOS kernel is self-contained in this repository.

## Building

```bash
./build_rpi2.sh
```

This creates `Build/kernel7.img` which can be booted directly on RPi2 hardware.

## Installation to SD Card

1. **Format SD card** as FAT32

2. **Download Raspberry Pi firmware** from:
   https://github.com/raspberrypi/firmware/tree/master/boot

   Required files:
   - `bootcode.bin`
   - `start.elf`
   - `fixup.dat`

3. **Copy files to SD card root**:
   ```bash
   cp bootcode.bin start.elf fixup.dat /mnt/sdcard/
   cp Build/kernel7.img /mnt/sdcard/
   ```

4. **Optional: Create config.txt**:
   ```
   kernel=kernel7.img
   enable_uart=1
   ```

5. **Boot**: Insert SD card into RPi2 and power on

## Memory Map

| Address      | Description                |
|--------------|----------------------------|
| 0x00000000   | GPU firmware / mailbox     |
| 0x00008000   | FreeRTOS kernel start      |
| 0x3F000000   | Peripheral base            |
| 0x3F201000   | UART0 (PL011)              |
| 0x3F215040   | Mini UART                  |
| 0x3F003000   | System timer               |
| 0x40000000   | ARM local peripherals      |

## Current Status

- ✅ Boot code and vector table
- ✅ Linker script for 0x8000 boot
- ✅ FreeRTOS kernel integrated
- ✅ Build system (self-contained)
- ✅ ARM generic timer configuration
- ✅ Minimal libc functions
- ✅ Compiles successfully (~36KB kernel)
- ✅ UART driver (PL011 at 0x3F201000)
- ✅ **TESTED ON HARDWARE - WORKING!**
- ✅ HYP mode detection and exit
- ✅ Secondary CPU parking
- ✅ FreeRTOS scheduler running
- ✅ Heap allocation working (32MB heap)

## Boot Sequence

The BCM2837 GPU firmware boots the ARM CPU in **HYP mode** (Hypervisor mode). The startup code:

1. **Parks secondary CPUs** (CPU1-3) immediately to prevent race conditions
2. **Detects HYP mode** and uses `eret` to drop to SVC (Supervisor) mode
3. **Skips FPU initialization** (not needed, causes faults)
4. **Sets up IRQ and SVC stacks** (8KB IRQ, 16KB main)
5. **Clears BSS section**
6. **Calls main()** to start FreeRTOS

Debug output during boot: `XYIVHYEN123456789` indicates successful boot.

## Hardware Testing

Confirmed working on Raspberry Pi 2B v1.2 hardware:
- Boot from SD card
- UART output via GPIO14/15 (115200 baud)
- FreeRTOS task creation and scheduling
- Memory allocation from 32MB heap

## Notes

- **kernel7.img**: RPi firmware looks for this filename for ARMv7/ARMv8-32 kernels
- **CPU parking**: Secondary cores (CPU1-3) are parked in startup code using WFI
- **FPU disabled**: Skipped to avoid undefined instruction faults (not needed for current application)
- **HYP mode**: GPU firmware boots in HYP mode, startup code drops to SVC mode for FreeRTOS
- **Heap size**: 32MB allocated for FreeRTOS (configurable in FreeRTOSConfig.h)
- **Boot address**: 0x8000 (loaded by GPU firmware)

## References

- [BCM2837 Peripherals](https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf)
- [RPi Bare Metal](https://github.com/bztsrc/raspi3-tutorial)
- [FreeRTOS ARM_CA9 port](https://www.freertos.org/Using-FreeRTOS-on-Cortex-A-Embedded-Processors.html)
