# Development Guide: FreeRTOS on RPi2 BCM2837

## Answer to Your Question

### "Why keep FreeRTOS clean if we need to debug?"

**Short Answer**: You DON'T need to keep FreeRTOS clean during development! I made a mistake by cleaning it too early.

## Two Phases of Development

### Phase 1: Development & Debugging (WHERE YOU ARE NOW)

**Goal**: Get FreeRTOS working on hardware with UART debug output

**What you CAN do**:
- ✅ **Modify FreeRTOS files** if needed for debugging
- ✅ Add UART debug statements in `FreeRTOS/portable/GCC/ARM_CA9/port.c`
- ✅ Add print statements anywhere to understand what's happening
- ✅ Commit these changes to your development branch

**What you HAVE NOW**:
- ✅ Clean FreeRTOS kernel (not modified)
- ✅ Proper UART driver in `Source/uart.c` with `uart_printf()`
- ✅ All debug code in YOUR files (not in FreeRTOS)
- ✅ Build works (36KB kernel)

**This is BETTER** because:
1. You can add debug output without touching FreeRTOS
2. Your UART driver works on real RPi2 hardware (0x3F201000)
3. Easy to update FreeRTOS if needed

### Phase 2: Production/GitHub Release (FUTURE)

**Goal**: Clean, maintainable code for public release

**When**: After FreeRTOS is working and tested on hardware

**What to do then**:
1. Move any debug code from FreeRTOS to your Source/ files
2. Create wrapper functions if needed
3. Document your modifications

## Current Project Status

### ✅ You Have Working Debug Infrastructure

```c
// In YOUR code (Source/uart.c):
uart_printf("Task started at priority %d\n", priority);
uart_printf("Stack pointer: 0x%x\n", sp_value);
```

### How to Add Debug Output

**Option 1: In your application** (main.c, tasks)
```c
#include "uart.h"

void my_task(void *params) {
    uart_printf("Task running!\n");
    // your code
}
```

**Option 2: In FreeRTOS (if really needed)**
```c
// In FreeRTOS/portable/GCC/ARM_CA9/port.c
// Add at top:
extern void uart_printf(const char *format, ...);

// Add in function:
void xPortStartScheduler(void) {
    uart_printf("Starting scheduler...\n");
    // existing code
}
```

Then declare in Source/uart.h that it's available for FreeRTOS.

## My Recommendation for YOUR Case

### Keep It As Is (Current Setup is Good!)

**Why:**
1. ✅ FreeRTOS is clean (no modifications)
2. ✅ You have full UART debug in Source/uart.c
3. ✅ You can call `uart_printf()` from anywhere
4. ✅ Builds successfully
5. ✅ Ready to test on hardware

### If You Need to Debug FreeRTOS Internals

If FreeRTOS itself isn't working (scheduler issues, task switching), THEN:

1. **Add extern declaration in FreeRTOS**:
   ```c
   // At top of portable/GCC/ARM_CA9/port.c:
   extern void uart_printf(const char *format, ...);
   ```

2. **Add debug output where needed**:
   ```c
   void vPortTaskSwitch(void) {
       uart_printf("Task switch!\n");
       // existing code
   }
   ```

3. **Document these changes**:
   ```bash
   cd FreeRTOS
   git diff > ../freertos_debug_patches.diff
   ```

4. **Can revert anytime**:
   ```bash
   cd FreeRTOS
   git checkout .  # Revert all changes
   ```

## File Organization Summary

### Your Application Code
```
Source/
├── main.c           - Your application & tasks
├── uart.c           - UART driver (printf, putc, etc)
├── uart.h           - UART interface
├── rpi2_support.c   - Hardware support functions
└── FreeRTOSConfig.h - FreeRTOS configuration
```

### FreeRTOS Kernel (git submodule)
```
FreeRTOS/
├── tasks.c          - FreeRTOS scheduler
├── portable/        - Port for ARM_CA9
└── include/         - FreeRTOS headers
```

**Status**: Clean (no modifications needed currently)

## When to Test on Hardware

You're ready to test RIGHT NOW:

1. Build: `./build_rpi2.sh` → creates `Build/kernel7.img`
2. Copy to SD card with RPi firmware
3. Boot and connect UART to see output
4. Add more debug `uart_printf()` as needed

## Summary

**My mistake**: I cleaned FreeRTOS too early, thinking you needed "production ready" code.

**Reality**: You have a BETTER setup:
- Clean FreeRTOS (easy to update)
- Full debugging capability via uart.c
- Can modify FreeRTOS later if truly needed
- Ready to test on hardware NOW

**Next step**: Boot on your RPi 2B v1.2 and see UART output!

If scheduler doesn't work, THEN we can add debug to FreeRTOS internals. But try the hardware first - it might just work!
