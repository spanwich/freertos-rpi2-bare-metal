# Setting Up FreeRTOS as a Git Submodule

This guide explains how to properly include FreeRTOS kernel in your repository **without modifying it**.

## Current Status

✅ **What we have**: FreeRTOS kernel copied into `FreeRTOS/` directory
❌ **Problem**: This is a full copy, hard to update, and not properly attributed

## Proper Solution: Use Git Submodule

### Step 1: Remove the Copied FreeRTOS

```bash
cd /home/iamfo470/phd/freertos_rpi2_bcm2837
rm -rf FreeRTOS
```

### Step 2: Add FreeRTOS as Submodule

```bash
# Add official FreeRTOS kernel as submodule
git submodule add https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS

# Initialize and fetch the submodule
git submodule update --init --recursive
```

### Step 3: Verify Structure

After adding the submodule, you should see:

```bash
$ ls FreeRTOS/
tasks.c  queue.c  list.c  timers.c  event_groups.c  stream_buffer.c
include/  portable/  ...
```

### Step 4: Commit the Submodule

```bash
git add .gitmodules FreeRTOS
git commit -m "Add FreeRTOS kernel as git submodule"
```

## Benefits of Git Submodule

1. ✅ **Proper Attribution**: FreeRTOS remains in its own repository
2. ✅ **Easy Updates**: `git submodule update --remote FreeRTOS`
3. ✅ **Version Control**: Pin to specific FreeRTOS version/tag
4. ✅ **Clean Separation**: Your code in `Source/`, FreeRTOS in `FreeRTOS/`
5. ✅ **License Compliance**: FreeRTOS license is preserved

## Alternative: Vendor the Kernel (Current Approach)

If you prefer to keep FreeRTOS copied (not as submodule):

### Pros:
- ✅ Self-contained repository (no submodules to clone)
- ✅ Works offline
- ✅ Guaranteed consistency

### Cons:
- ❌ Harder to update FreeRTOS
- ❌ Larger repository size
- ❌ Must manually track FreeRTOS version

### How to Vendor Properly:

1. **Add a LICENSE file** referencing FreeRTOS MIT license
2. **Document the version**:
   ```bash
   # Add to README.md:
   This repository includes FreeRTOS Kernel v10.5.1
   https://github.com/FreeRTOS/FreeRTOS-Kernel
   Licensed under MIT License
   ```

3. **Never modify FreeRTOS kernel files**
   - All your code goes in `Source/`
   - Configuration in `Source/FreeRTOSConfig.h`

## Recommended Approach for Your Use Case

**Use Git Submodule** because:
- You're doing research (want latest features)
- You might need to update FreeRTOS
- Proper academic attribution
- Standard practice in embedded projects

## When Cloning Your Repository (for others)

If you use submodules, others must clone with:

```bash
# Clone with submodules
git clone --recursive https://github.com/YOUR_USERNAME/freertos-rpi2-bare-metal.git

# Or if already cloned:
git submodule update --init --recursive
```

## File Organization (Final Structure)

```
freertos-rpi2-bare-metal/
├── Source/                 # YOUR code (tracked in YOUR repo)
│   ├── main.c
│   ├── rpi2_support.c
│   └── FreeRTOSConfig.h
├── FreeRTOS/              # Git submodule (tracked in FreeRTOS repo)
│   ├── tasks.c           # DO NOT MODIFY
│   ├── queue.c           # DO NOT MODIFY
│   ├── include/          # DO NOT MODIFY
│   └── portable/         # DO NOT MODIFY
├── Startup/               # YOUR code
├── Build/                 # Ignored by git
├── build_rpi2.sh         # YOUR code
└── README.md             # YOUR code
```

## Summary

**Rule of Thumb**:
- Everything in `Source/` and `Startup/` = YOUR code = commit to your repo
- Everything in `FreeRTOS/` = FreeRTOS kernel = git submodule or vendored
- Everything in `Build/` = build artifacts = ignored by git
