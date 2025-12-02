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
├── FreeRTOSConfig.h        # FreeRTOS configuration for BCM2837
├── Startup/
│   ├── startup_rpi2.S      # Boot code and vector table
│   └── link_rpi2.ld        # Linker script (boots at 0x8000)
├── Build/                  # Build output directory
├── build_rpi2.sh          # Build script
└── README.md              # This file
```

## Dependencies

- FreeRTOS source (reuses from `../freertos_vexpress_a9/Source`)
- ARM cross-compiler: `arm-none-eabi-gcc`
- Raspberry Pi firmware files (bootcode.bin, start.elf, fixup.dat)

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
- ✅ FreeRTOS configuration
- ✅ Build script
- ⚠️ UART driver (TODO)
- ⚠️ Interrupt controller driver (TODO)
- ⚠️ Timer driver (TODO)

## Next Steps

To make this functional, you need to implement:

1. **UART driver** for console output (PL011 at 0x3F201000)
2. **Interrupt controller** (BCM2836 ARM control at 0x3F00B200)
3. **Timer configuration** (ARM generic timer or system timer)

## Notes

- **kernel7.img**: RPi firmware looks for this filename for ARMv7/ARMv8-32 kernels
- **CPU parking**: Secondary cores (CPU1-3) are parked in startup code
- **FPU enabled**: VFPv4 with NEON is initialized in startup
- **Heap size**: 32MB allocated for FreeRTOS (configurable in FreeRTOSConfig.h)

## References

- [BCM2837 Peripherals](https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf)
- [RPi Bare Metal](https://github.com/bztsrc/raspi3-tutorial)
- [FreeRTOS ARM_CA9 port](https://www.freertos.org/Using-FreeRTOS-on-Cortex-A-Embedded-Processors.html)
