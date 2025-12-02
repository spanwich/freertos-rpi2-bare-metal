#!/bin/bash
# Build script for FreeRTOS on Raspberry Pi 2B v1.2 (BCM2837)
# Creates kernel7.img for bare-metal boot

set -e  # Exit on any error

echo "=========================================="
echo "  FreeRTOS RPi2 BCM2837 Build Script"
echo "=========================================="

# Paths
FREERTOS_SRC="./FreeRTOS"
BUILD_DIR="Build"
STARTUP_DIR="Startup"
OUTPUT="kernel7.img"

# Check if FreeRTOS source exists
if [ ! -d "$FREERTOS_SRC" ]; then
    echo "ERROR: FreeRTOS source not found at $FREERTOS_SRC"
    exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Cleaning previous build..."
rm -f *.o *.elf *.img

# Compiler flags for Cortex-A53 in AArch32 mode
CFLAGS="-mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard -marm"
CFLAGS="$CFLAGS -nostdlib -ffreestanding -O2 -Wall"
CFLAGS="$CFLAGS -I.. -I$FREERTOS_SRC/include -I$FREERTOS_SRC/portable/GCC/ARM_CA9"

ASFLAGS="-mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard"

LDFLAGS="-T../$STARTUP_DIR/link_rpi2.ld -nostdlib -lgcc"

# Assemble startup code
echo "Assembling startup code..."
arm-none-eabi-gcc $ASFLAGS -c -o startup.o ../$STARTUP_DIR/startup_rpi2.S

# Compile main application (use existing or create minimal one)
if [ -f "$FREERTOS_SRC/main.c" ]; then
    echo "Compiling main application..."
    arm-none-eabi-gcc $CFLAGS -c -o main.o "$FREERTOS_SRC/main.c"
else
    echo "WARNING: No main.c found, creating minimal main..."
    cat > main_minimal.c <<'EOF'
#include "FreeRTOS.h"
#include "task.h"

void vAssertCalled(const char *pcFile, unsigned long ulLine) {
    while(1) {}  // Hang on assertion
}

void vApplicationMallocFailedHook(void) {
    while(1) {}  // Hang on malloc failure
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    while(1) {}  // Hang on stack overflow
}

static void prvTestTask(void *pvParameters) {
    (void)pvParameters;
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    // Create test task
    xTaskCreate(prvTestTask, "Test", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    // Start scheduler
    vTaskStartScheduler();

    // Should never reach here
    while(1) {}
    return 0;
}
EOF
    arm-none-eabi-gcc $CFLAGS -c -o main.o main_minimal.c
fi

# Compile FreeRTOS core
echo "Compiling FreeRTOS core..."
for source in tasks.c queue.c list.c timers.c event_groups.c stream_buffer.c; do
    echo "  Compiling $source..."
    arm-none-eabi-gcc $CFLAGS -c -o ${source%.c}.o "$FREERTOS_SRC/$source"
done

# Compile FreeRTOS port
echo "Compiling FreeRTOS ARM_CA9 port..."
arm-none-eabi-gcc $CFLAGS -c -o port.o "$FREERTOS_SRC/portable/GCC/ARM_CA9/port.c"
arm-none-eabi-gcc $ASFLAGS -c -o portASM.o "$FREERTOS_SRC/portable/GCC/ARM_CA9/portASM.S"

# Compile heap implementation
echo "Compiling heap_4..."
arm-none-eabi-gcc $CFLAGS -c -o heap_4.o "$FREERTOS_SRC/portable/MemMang/heap_4.c"

# Link everything
echo "Linking..."
arm-none-eabi-gcc $LDFLAGS -o freertos.elf \
    startup.o \
    main.o \
    tasks.o \
    queue.o \
    list.o \
    timers.o \
    event_groups.o \
    stream_buffer.o \
    heap_4.o \
    port.o \
    portASM.o

# Convert to binary (kernel7.img for RPi2/3)
echo "Creating kernel7.img..."
arm-none-eabi-objcopy freertos.elf -O binary kernel7.img

# Show size info
echo ""
echo "Build completed successfully!"
echo ""
arm-none-eabi-size freertos.elf
echo ""
echo "Output file: $BUILD_DIR/kernel7.img"
echo ""
echo "To boot on Raspberry Pi 2B v1.2:"
echo "1. Format SD card as FAT32"
echo "2. Copy RPi firmware files (bootcode.bin, start.elf, fixup.dat)"
echo "3. Copy kernel7.img to SD card"
echo "4. Optional: Create config.txt with 'kernel=kernel7.img'"
echo "5. Insert SD card and power on"
