# Contributing Guidelines

## Golden Rule: NEVER Modify FreeRTOS Kernel

### ✅ Correct Approach

**All your code goes in `Source/` directory:**

```
Source/
├── main.c              # Your application
├── rpi2_support.c      # Your hardware drivers
├── uart.c              # Your UART driver (if needed)
└── FreeRTOSConfig.h    # Your FreeRTOS configuration
```

### ❌ NEVER Do This

**DO NOT modify files in `FreeRTOS/` directory:**
- ❌ `FreeRTOS/portable/GCC/ARM_CA9/port.c`
- ❌ `FreeRTOS/portable/GCC/ARM_CA9/portASM.S`
- ❌ `FreeRTOS/tasks.c`
- ❌ Any file in `FreeRTOS/`

## Why?

1. **FreeRTOS is a git submodule** - modifications will conflict with updates
2. **License compliance** - modifying FreeRTOS requires different license handling
3. **Maintainability** - your changes will be lost when updating FreeRTOS
4. **Academic integrity** - unclear what's your work vs. FreeRTOS code

## How to Verify FreeRTOS is Clean

Before committing:

```bash
cd FreeRTOS
git status
# Should show: "nothing to commit, working tree clean"
```

If you see modified files:

```bash
# Revert all changes
git checkout .

# Remove any .o files you created
find . -name "*.o" -delete
```

## Project File Organization

### Your Code (commit to your repo)
```
Source/                 ✅ YOUR code - modify freely
Startup/                ✅ YOUR code - modify freely
build_rpi2.sh          ✅ YOUR script - modify freely
README.md              ✅ YOUR docs - modify freely
```

### FreeRTOS Kernel (git submodule - DO NOT MODIFY)
```
FreeRTOS/              ❌ Git submodule - never modify
├── .git → ../.git/modules/FreeRTOS
├── tasks.c            ❌ DO NOT MODIFY
├── portable/          ❌ DO NOT MODIFY
└── include/           ❌ DO NOT MODIFY
```

### Build Artifacts (ignored by git)
```
Build/                 ⚠️ Build output - never commit
├── *.o
├── *.elf
└── kernel7.img
```

## Adding Features

### Example: Adding UART Debug Output

**❌ WRONG WAY:**
```c
// Editing FreeRTOS/portable/GCC/ARM_CA9/port.c
#include "FreeRTOS.h"

void uart_putc(char c) { ... }  // ❌ Don't add this here!
```

**✅ CORRECT WAY:**

Create `Source/uart.c`:
```c
// Source/uart.c
#include "FreeRTOS.h"
#include "uart.h"

void uart_putc(char c) {
    // Your UART implementation
}
```

Create `Source/uart.h`:
```c
// Source/uart.h
#ifndef UART_H
#define UART_H

void uart_putc(char c);
void uart_puts(const char *s);

#endif
```

Use in `Source/main.c`:
```c
#include "uart.h"

int main(void) {
    uart_puts("Hello from RPi2!\n");
    // ...
}
```

## Before Pushing to GitHub

### Checklist:

1. ✅ Verify FreeRTOS is clean:
   ```bash
   cd FreeRTOS && git status
   # Output: "nothing to commit, working tree clean"
   ```

2. ✅ Verify only your code is staged:
   ```bash
   git status
   # Should only show: Source/, Startup/, build_rpi2.sh, README.md
   ```

3. ✅ Build succeeds:
   ```bash
   rm -rf Build && ./build_rpi2.sh
   ```

4. ✅ No build artifacts committed:
   ```bash
   git status
   # Should NOT show: Build/, *.o, *.elf
   ```

## Summary

| Directory | Action | Reason |
|-----------|--------|--------|
| `Source/` | ✅ Modify freely | Your application code |
| `Startup/` | ✅ Modify freely | Your boot code |
| `FreeRTOS/` | ❌ Never modify | Git submodule (FreeRTOS kernel) |
| `Build/` | ⚠️ Never commit | Temporary build artifacts |

**Remember:** If you need to customize FreeRTOS behavior, use `Source/FreeRTOSConfig.h` configuration options or create wrapper functions in `Source/`.
