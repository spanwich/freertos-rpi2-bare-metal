# SD Card Setup for Bare-Metal FreeRTOS on RPi2

## Quick Answer

**Q: Can I boot FreeRTOS directly?**
**A: YES, but you need Raspberry Pi GPU firmware files first.**

## Why You Need Firmware Files

### Raspberry Pi Boot Sequence:
```
Power On → GPU starts (not ARM!) → GPU loads firmware from SD →
GPU loads YOUR kernel (kernel7.img) → GPU starts ARM CPU →
ARM CPU runs YOUR FreeRTOS code
```

**The GPU is in control** until it hands off to your kernel!

## What Goes on the SD Card

### Minimum Files Needed:

```
/dev/mmcblk0p1 (FAT32 boot partition):
├── bootcode.bin         ← GPU first stage bootloader (REQUIRED)
├── start.elf            ← GPU firmware (REQUIRED)
├── fixup.dat            ← GPU memory config (REQUIRED)
├── kernel7.img          ← YOUR FreeRTOS! (this is what we built)
└── config.txt           ← Boot configuration (OPTIONAL)
```

**Your FreeRTOS (`kernel7.img`) IS booting directly** - but the GPU firmware gets it there.

## SD Card Setup Steps

### Step 1: Check Your SD Card

```bash
# Check if SD card is detected
lsblk | grep mmcblk0

# Output should show something like:
# mmcblk0      179:0    0  14.8G  0 disk
# └─mmcblk0p1  179:1    0   256M  0 part  (boot partition)
```

### Step 2: Create Boot Partition (if needed)

If your SD card is blank:

```bash
# Create partition table
sudo fdisk /dev/mmcblk0
# Press: o (new DOS partition table)
# Press: n (new partition)
# Press: p (primary)
# Press: 1 (partition 1)
# Press: Enter (default start)
# Press: Enter (use whole disk)
# Press: t (change type)
# Press: c (W95 FAT32 LBA)
# Press: w (write and exit)

# Format as FAT32
sudo mkfs.vfat /dev/mmcblk0p1
```

### Step 3: Mount SD Card

```bash
# Create mount point
sudo mkdir -p /mnt/rpi_boot

# Mount the boot partition
sudo mount /dev/mmcblk0p1 /mnt/rpi_boot
```

### Step 4: Download RPi Firmware

**Option A: Download official files**
```bash
cd /tmp
wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat

# Copy to SD card
sudo cp bootcode.bin start.elf fixup.dat /mnt/rpi_boot/
```

**Option B: Use existing Raspbian SD card**
```bash
# If you have a working Raspbian SD card, just copy the files:
sudo cp /path/to/raspbian/bootcode.bin /mnt/rpi_boot/
sudo cp /path/to/raspbian/start.elf /mnt/rpi_boot/
sudo cp /path/to/raspbian/fixup.dat /mnt/rpi_boot/
```

### Step 5: Copy Your FreeRTOS Kernel

```bash
# Copy your built kernel
sudo cp Build/kernel7.img /mnt/rpi_boot/

# Verify it's there
ls -lh /mnt/rpi_boot/
```

### Step 6: Create config.txt (Recommended)

```bash
# Create config file
sudo tee /mnt/rpi_boot/config.txt <<EOF
# Bare-metal FreeRTOS configuration for RPi2B v1.2 (BCM2837)

# Boot our kernel
kernel=kernel7.img

# Enable UART for debug output
enable_uart=1
uart_2ndstage=1

# Disable Linux-specific features
avoid_warnings=1

# CPU frequency (900MHz for BCM2837 on RPi2 v1.2)
arm_freq=900
core_freq=250

# Memory split (give all memory to ARM, none to GPU)
gpu_mem=16
EOF
```

### Step 7: Unmount and Boot

```bash
# Sync to ensure all writes complete
sudo sync

# Unmount
sudo umount /mnt/rpi_boot

# Remove SD card and insert into RPi2
```

## What Your SD Card Should Look Like

```bash
# Check contents
ls -lh /mnt/rpi_boot/

# Should show:
# -rwxr-xr-x  1 root root  52K  bootcode.bin    ← GPU bootloader
# -rwxr-xr-x  1 root root 2.8M  start.elf       ← GPU firmware
# -rwxr-xr-x  1 root root 6.7K  fixup.dat       ← GPU config
# -rwxr-xr-x  1 root root  47K  kernel7.img     ← YOUR FreeRTOS!!!
# -rwxr-xr-x  1 root root 200   config.txt      ← Boot config
```

## Complete One-Command Setup

```bash
#!/bin/bash
# Quick setup script

BOOT_PART="/dev/mmcblk0p1"
MOUNT_DIR="/mnt/rpi_boot"

# Mount
sudo mkdir -p $MOUNT_DIR
sudo mount $BOOT_PART $MOUNT_DIR

# Download firmware (if not already there)
cd /tmp
[ ! -f bootcode.bin ] && wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
[ ! -f start.elf ] && wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
[ ! -f fixup.dat ] && wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat

# Copy everything
sudo cp bootcode.bin start.elf fixup.dat $MOUNT_DIR/
sudo cp ~/phd/freertos_rpi2_bcm2837/Build/kernel7.img $MOUNT_DIR/

# Create config
sudo tee $MOUNT_DIR/config.txt <<EOF
kernel=kernel7.img
enable_uart=1
gpu_mem=16
EOF

# Done
sudo sync
sudo umount $MOUNT_DIR
echo "SD card ready! Insert into RPi2 and boot."
```

## Understanding kernel7.img

**Q: Why kernel7.img and not kernel.img?**

| Filename | RPi Model | CPU |
|----------|-----------|-----|
| `kernel.img` | RPi1 (BCM2835) | ARM1176 (ARMv6) |
| `kernel7.img` | **RPi2/3 (BCM2836/7)** | **Cortex-A7/A53 (ARMv7/v8-32)** |
| `kernel8.img` | RPi3/4 (64-bit mode) | Cortex-A53/A72 (ARMv8-64) |

**Your RPi2 v1.2 with BCM2837** runs in 32-bit mode, so it uses `kernel7.img`.

## Testing Connection

### UART Connection (for debug output)

Connect USB-to-UART adapter:
- **TX** (RPi GPIO14) → RX on adapter
- **RX** (RPi GPIO15) → TX on adapter
- **GND** (RPi GND) → GND on adapter

On your PC:
```bash
# Find the serial device
ls /dev/ttyUSB* /dev/ttyACM*

# Connect (115200 baud, 8N1)
screen /dev/ttyUSB0 115200

# Or with minicom
minicom -D /dev/ttyUSB0 -b 115200
```

You should see output from `uart_printf()` calls!

## Summary

**You ARE booting FreeRTOS directly** - the firmware files just get the hardware to that point.

Think of it like BIOS on a PC:
- PC: BIOS → loads OS kernel
- RPi: GPU firmware → loads YOUR kernel (FreeRTOS)

**Minimum SD card contents**:
```
bootcode.bin  ← Get from Raspberry Pi firmware repo
start.elf     ← Get from Raspberry Pi firmware repo
fixup.dat     ← Get from Raspberry Pi firmware repo
kernel7.img   ← YOUR FreeRTOS (Build/kernel7.img)
```

That's it! No Linux, no nothing - just FreeRTOS running on bare metal.
