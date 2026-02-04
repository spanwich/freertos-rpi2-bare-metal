#!/bin/bash
# Minimal UART test build - no FreeRTOS, just startup + UART + test main
set -e

BUILD_DIR="Build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
rm -f uart_test.elf kernel7.img

CFLAGS="-mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard -marm -nostdlib -ffreestanding -O2 -Wall -I../Source"
ASFLAGS="-mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard"
LDFLAGS="-T../Startup/link_rpi2.ld -nostdlib -lgcc"

echo "Building minimal UART test..."
arm-none-eabi-gcc $ASFLAGS -c -o startup.o ../Startup/startup_rpi2.S
arm-none-eabi-gcc $CFLAGS -c -o uart.o ../Source/uart.c
arm-none-eabi-gcc $CFLAGS -c -o main_uart_test.o ../Source/main_uart_test.c

echo "Linking..."
arm-none-eabi-gcc $LDFLAGS -o uart_test.elf startup.o uart.o main_uart_test.o

echo "Creating kernel7.img..."
arm-none-eabi-objcopy uart_test.elf -O binary kernel7.img

arm-none-eabi-size uart_test.elf
echo "Done! Copy Build/kernel7.img to SD card."
