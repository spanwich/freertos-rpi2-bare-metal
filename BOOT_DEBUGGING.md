# Boot Debugging: RPi2 BCM2837 FreeRTOS

This document describes the boot issues encountered and how they were resolved.

## Hardware Boot Behavior

The Raspberry Pi 2B v1.2 (BCM2837) has unique boot characteristics:

1. **GPU firmware boots first** - VideoCore GPU loads from SD card
2. **GPU loads ARM kernel** - Reads kernel7.img to address 0x8000
3. **GPU starts ARM in HYP mode** - Hypervisor mode, not SVC mode
4. **All 4 CPU cores start** - CPU0-3 all execute from 0x8000 simultaneously
5. **No bootloader** - Direct jump to 0x8000 with minimal initialization

## Boot Issues Encountered

### Issue 1: System Stopped at "SC" Output

**Symptom**: UART output showed GPU firmware messages ending with "SDCARD_CONTROL_POWER not defined" followed by "SC"

**Root cause**: The 'S' was from our debug code, but 'C' was GPU firmware message bleeding through, NOT from our code.

**Debug approach**: Changed debug characters from 'S'/'C' to 'X'/'Y' to verify source

**Result**: Output changed to "XY", confirming kernel was executing but hanging immediately after

### Issue 2: Crash After First Debug Output

**Symptom**: System printed "XY" and hung

**Root cause**: Multiple CPU cores executing same code simultaneously, causing race conditions

**Solution**: Move CPU parking code to **very first instruction** in `_start:`

```assembly
_start:
    @ Park secondary CPUs IMMEDIATELY (before ANY other code)
    mrc p15, 0, r0, c0, c0, 5    @ Read MPIDR
    ands r0, r0, #3              @ Extract CPU ID (bits 0-1)
    bne cpu_park                 @ If not CPU0, park
```

**Result**: Now only CPU0 continues past this point

### Issue 3: Crash During Mode Switch

**Symptom**: System printed "XYIV1" and hung when trying `cps #0x12` (switch to IRQ mode)

**Root cause**: BCM2837 GPU firmware boots ARM CPU in **HYP mode** (Hypervisor mode). The `cps` instruction cannot be used to change modes from HYP mode.

**Investigation**: Added mode detection code to check CPSR register:
```assembly
mrs r0, cpsr                 @ Read current mode
and r0, r0, #0x1F            @ Extract mode bits (bottom 5 bits)
cmp r0, #0x1A                @ Check if HYP mode (0x1A = 26 decimal)
```

**Result**: Confirmed CPU was in HYP mode (0x1A)

### Issue 4: HYP Mode Exit Failure

**Symptom**: System detected HYP mode ("XYIVHY") but crashed during exit attempt

**Root cause**: Initial HYP exit code used `msr elr_hyp, pc` which has undefined behavior

**Original broken code**:
```assembly
@ BROKEN - don't use pc with msr
msr elr_hyp, pc              @ Return to next instruction
eret                         @ Drop to SVC mode
```

**Solution**: Use `adr` to get a proper return address:
```assembly
@ CORRECT - use adr to get address label
mov r0, #0xD3                @ SVC mode (0x13) + IRQ/FIQ disabled (0xC0)
msr spsr_hyp, r0             @ Set return mode to SVC
adr lr, hyp_exit             @ Load address of hyp_exit label
msr elr_hyp, lr              @ Set exception return address
eret                         @ Drop to SVC mode

hyp_exit:
    @ Successfully exited HYP mode, now in SVC mode
```

**Result**: Successfully drops from HYP to SVC mode ("XYIVHYE")

### Issue 5: FPU Initialization Fault

**Symptom**: System would crash when trying to initialize FPU

**Root cause**: GPU firmware doesn't properly enable VFP/NEON before handoff

**Solution**: Skip FPU initialization entirely (not needed for memory painting application)

```assembly
@ Skip FPU initialization - not needed for memory painting
@ Enabling FPU causes undefined instruction fault on this platform
@ and adds unnecessary context-switching overhead
```

**Benefit**: Removes context-switching overhead and avoids fault

## Final Boot Sequence

Successful boot shows debug output: **`XYIVHYEN123456789`**

### Debug Character Meanings

| Char | Stage | Description |
|------|-------|-------------|
| X | Entry | Kernel started execution |
| Y | CPU0 | CPU0 confirmed (others parked) |
| I | Interrupts | Interrupts disabled |
| V | VBAR | Vector table base address set |
| H | HYP Check | Mode check started |
| Y | HYP Detected | CPU is in HYP mode |
| E | HYP Exit | Successfully exited to SVC mode |
| N | Not HYP | Now in non-HYP (SVC) mode |
| 1 | IRQ Setup | Before IRQ mode switch |
| 2 | IRQ Mode | After IRQ mode switch |
| 3 | IRQ Stack | IRQ stack pointer set |
| 4 | SVC Mode | After SVC mode switch |
| 5 | SVC Stack | SVC stack pointer set |
| 6 | BSS Start | BSS start address loaded |
| 7 | BSS End | BSS end address loaded |
| 8 | BSS Clear | BSS section zeroed |
| 9 | Main Call | About to call main() |

After '9', the system jumps to main() and FreeRTOS starts.

## Key Lessons Learned

### 1. Multi-Core Boot Requires Immediate CPU Parking

**Problem**: ARMv7/ARMv8 systems boot with all cores active

**Solution**: First instruction must identify and park secondary cores:
```assembly
_start:
    mrc p15, 0, r0, c0, c0, 5    @ Read MPIDR (CPU ID register)
    ands r0, r0, #3              @ Extract CPU ID
    bne cpu_park                 @ Park if not CPU0
```

### 2. HYP Mode Requires Special Handling

**Problem**: `cps` instruction doesn't work from HYP mode

**Solution**: Use exception return (`eret`) to drop privilege:
```assembly
mov r0, #0xD3                @ Target mode (SVC + IRQ/FIQ disabled)
msr spsr_hyp, r0             @ Set saved program status
adr lr, target               @ Get return address
msr elr_hyp, lr              @ Set exception link register
eret                         @ Exception return (drops to target mode)
```

### 3. FPU Initialization May Not Be Possible

**Problem**: GPU firmware doesn't enable coprocessor access

**Solution**: Skip FPU if not needed (saves context-switch overhead)

### 4. Debug Characters Essential for Bare-Metal

**Problem**: No debugger available on bare-metal hardware

**Solution**: Single-character UART output at each boot stage provides precise fault location

### 5. Don't Trust Mixed Output Sources

**Problem**: "SC" output looked like our code but was mixed with GPU firmware

**Solution**: Use unique, unlikely character sequences to verify output source

## ARM Processor Modes

### Mode Numbers (CPSR bits [4:0])

| Mode | Value | Description |
|------|-------|-------------|
| USR | 0x10 | User mode (unprivileged) |
| FIQ | 0x11 | Fast Interrupt Request |
| IRQ | 0x12 | Interrupt Request |
| SVC | 0x13 | Supervisor mode |
| ABT | 0x17 | Abort mode |
| UND | 0x1B | Undefined instruction |
| SYS | 0x1F | System mode (privileged) |
| **HYP** | **0x1A** | **Hypervisor mode** (virtualization) |

### Why FreeRTOS Needs SVC Mode

- FreeRTOS uses `svc` (supervisor call) instruction for system calls
- Task switching requires SVC mode privileges
- Cannot use `svc` from HYP mode (generates hypervisor trap instead)
- Must drop to SVC mode before starting scheduler

## References

- ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition
- BCM2837 ARM-local peripherals specification
- Raspberry Pi firmware boot sequence documentation
- FreeRTOS ARM Cortex-A port documentation

## Boot Code Location

All startup code is in: `/home/iamfo470/phd/freertos_rpi2_bcm2837/Startup/startup_rpi2.S`
