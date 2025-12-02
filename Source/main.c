#include "FreeRTOS.h"
#include "task.h"
#include <stddef.h>
#include <stdint.h>

// PL011 UART registers - matching the original vm_freertos example
#define UART0_DR (*(volatile unsigned int *)0x9000000)   // Data register
#define UART0_FR (*(volatile unsigned int *)0x9000018)   // Flag register

void uart_putc(char c) {
    // Better implementation with delay like original example
    UART0_DR = c;
    for (volatile int i = 0; i < 10000; i++) {}  // Simple delay
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

void uart_decimal(unsigned long val) {
    if (val == 0) {
        uart_putc('0');
        return;
    }
    
    char buffer[12]; // enough for 32-bit number
    int i = 0;
    while (val > 0) {
        buffer[i++] = '0' + (val % 10);
        val /= 10;
    }
    
    // Print in reverse order
    while (i > 0) {
        uart_putc(buffer[--i]);
    }
}

void uart_hex(unsigned int val) {
    int i;
    for (i = 28; i >= 0; i -= 4) {
        int digit = (val >> i) & 0xF;
        uart_putc(digit < 10 ? '0' + digit : 'A' + digit - 10);
    }
}

// Delay function from original example
void delay(volatile unsigned int count) {
    while (count--) {
        for (volatile int i = 0; i < 100000; i++) {}  // Reduced for faster testing
    }
}

// Message printing functions from original example
void print_freertos_starting(void) {
    uart_putc('F'); uart_putc('r'); uart_putc('e'); uart_putc('e');
    uart_putc('R'); uart_putc('T'); uart_putc('O'); uart_putc('S');
    uart_putc(' '); uart_putc('s'); uart_putc('t'); uart_putc('a');
    uart_putc('r'); uart_putc('t'); uart_putc('i'); uart_putc('n');
    uart_putc('g'); uart_putc('.'); uart_putc('.'); uart_putc('.');
    uart_putc('\n');
}

void print_hello_message(void) {
    uart_putc('H'); uart_putc('e'); uart_putc('l'); uart_putc('l');
    uart_putc('o'); uart_putc(' '); uart_putc('f'); uart_putc('r');
    uart_putc('o'); uart_putc('m'); uart_putc(' '); uart_putc('F');
    uart_putc('r'); uart_putc('e'); uart_putc('e'); uart_putc('R');
    uart_putc('T'); uart_putc('O'); uart_putc('S'); uart_putc('!');
    uart_putc('\n');
}

// Simple printf replacement
int printf(const char *format, ...) {
    uart_puts(format);
    return 0;
}

// Standard C library functions
void *memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void *memset(void *s, int c, size_t n) {
    char *p = (char *)s;
    for (size_t i = 0; i < n; i++) {
        p[i] = c;
    }
    return s;
}

void vAssertCalled(unsigned long ulLine, const char * const pcFileName) {
    uart_puts("\r\n=== DETAILED ASSERT FAILURE DEBUG ===\r\n");
    uart_puts("ASSERT FAILED at line: ");
    uart_decimal(ulLine);
    uart_puts("\r\n");
    uart_puts("File: ");
    uart_puts(pcFileName);
    uart_puts("\r\n");
    
    // Provide specific debugging for common assertion locations
    // Simple check for "port.c" in filename
    int isPortC = 0;
    const char *p = pcFileName;
    while (*p) {
        if (*p == 'p' && *(p+1) == 'o' && *(p+2) == 'r' && *(p+3) == 't' && *(p+4) == '.') {
            isPortC = 1;
            break;
        }
        p++;
    }
    
    if (isPortC) {
        uart_puts("\r\n--- PORT.C ASSERTION ANALYSIS ---\r\n");
        if (ulLine >= 410 && ulLine <= 420) {
            uart_puts("CPU Mode assertion - checking APSR register\r\n");
        } else if (ulLine >= 430 && ulLine <= 450) {
            uart_puts("GIC Binary Point Register assertion\r\n");
        } else if (ulLine >= 470 && ulLine <= 480) {
            uart_puts("Critical nesting assertion\r\n");
        } else if (ulLine >= 490 && ulLine <= 500) {
            uart_puts("Interrupt nesting assertion\r\n");
        } else {
            uart_puts("Other port.c assertion at line ");
            uart_decimal(ulLine);
            uart_puts("\r\n");
        }
    }
    
    uart_puts("\r\nSystem will halt here for debugging.\r\n");
    uart_puts("=====================================\r\n");
    for (;;);
}

void vSetupTickInterrupt(void) {
    // Simple stub - in a real system this would configure the timer
    // For now just placeholder
    uart_puts("vSetupTickInterrupt called - timer stub\r\n");
}

// Memory pattern painting task
void vMemoryPatternTask(void *pvParameters) {
    static unsigned int pattern_counter = 0;
    
    // Define memory region to paint (1MB starting at heap area)
    volatile uint32_t *memory_base = (volatile uint32_t *)0x42000000;  // Safe area after guest base
    const size_t memory_size = 1024 * 1024; // 1MB
    const size_t word_count = memory_size / sizeof(uint32_t);
    
    uart_puts("=== MEMORY PATTERN PAINTING TASK ===\r\n");
    uart_puts("Memory base: 0x");
    uart_hex((unsigned int)memory_base);
    uart_puts("\r\n");
    uart_puts("Size: ");
    uart_decimal(memory_size);
    uart_puts(" bytes (");
    uart_decimal(word_count);
    uart_puts(" words)\r\n");
    
    for (;;) {
        // Create different patterns each iteration
        uint32_t pattern;
        const char *pattern_name;
        
        switch (pattern_counter % 4) {
            case 0:
                pattern = 0xDEADBEEF;
                pattern_name = "DEADBEEF";
                break;
            case 1:
                pattern = 0xCAFEBABE;
                pattern_name = "CAFEBABE";
                break;
            case 2:
                pattern = 0x12345678;
                pattern_name = "12345678";
                break;
            case 3:
                pattern = 0xAA55AA55;
                pattern_name = "AA55AA55";
                break;
        }
        
        uart_puts("Painting memory with pattern: 0x");
        uart_puts(pattern_name);
        uart_puts("\r\n");
        
        // Paint memory with pattern
        for (size_t i = 0; i < word_count; i++) {
            memory_base[i] = pattern;
            
            // Progress indicator every 64K words
            if ((i % (16384)) == 0) {
                uart_puts("Progress: ");
                uart_decimal((i * 100) / word_count);
                uart_puts("%\r\n");
            }
        }
        
        uart_puts("Memory painting complete. Pattern: 0x");
        uart_puts(pattern_name);
        uart_puts("\r\n");
        
        // Verify a few locations
        uart_puts("Verification samples:\r\n");
        for (int j = 0; j < 5; j++) {
            size_t offset = j * (word_count / 5);
            uart_puts("  [");
            uart_decimal(offset);
            uart_puts("]: 0x");
            uart_hex(memory_base[offset]);
            uart_puts("\r\n");
        }
        
        pattern_counter++;
        
        // Wait longer to allow memory dump
        uart_puts("Waiting 10 seconds for memory dump...\r\n");
        vTaskDelay(pdMS_TO_TICKS(10000));  // 10 second delay
    }
}

void vPLCMain(void *pvParameters) {
    unsigned int counter = 0;
    
    for (;;) {
        print_hello_message();
        
        // Print a counter to show task is running
        uart_puts("PLC Task Counter: ");
        // Simple counter display (0-9)
        uart_putc('0' + (counter % 10));
        uart_puts("\r\n");
        
        counter++;
        vTaskDelay(pdMS_TO_TICKS(5000));  // 5 second delay (longer to not interfere)
    }
}

// Demo task similar to original example
void vDemoTask(void *pvParameters) {
    for (;;) {
        uart_puts("Demo task: FreeRTOS on seL4 microkernel!\r\n");
        vTaskDelay(pdMS_TO_TICKS(3000));  // 3 second delay
    }
}

int main(void) {
    uart_puts("=== MAIN() ENTRY POINT ===\r\n");
    print_freertos_starting();
    
    uart_puts("=== ABOUT TO INITIALIZE FREERTOS ===\r\n");
    uart_puts("Initializing FreeRTOS on seL4...\r\n");
    uart_puts("Creating tasks...\r\n");
    
    uart_puts("vPLCMain function address: 0x");
    uart_hex((unsigned int)vPLCMain);
    uart_puts("\r\n");
    uart_puts("vDemoTask function address: 0x");
    uart_hex((unsigned int)vDemoTask);
    uart_puts("\r\n");
    
    // Check initial heap status
    uart_puts("=== HEAP STATUS BEFORE TASK CREATION ===\r\n");
    
    uart_puts("configTOTAL_HEAP_SIZE: ");
    uart_decimal(configTOTAL_HEAP_SIZE);
    uart_puts(" bytes\r\n");
    
    // Test basic memory access before trying FreeRTOS heap
    uart_puts("Testing basic memory access...\r\n");
    volatile uint32_t *test_addr = (volatile uint32_t *)0x4001D000;  // Should be in BSS area
    *test_addr = 0x12345678;
    uint32_t read_val = *test_addr;
    uart_puts("Memory test: wrote 0x12345678, read 0x");
    uart_hex(read_val);
    uart_puts("\r\n");
    
    if (read_val == 0x12345678) {
        uart_puts("Basic memory access: SUCCESS\r\n");
        
        // Now try FreeRTOS heap allocation
        uart_puts("Testing FreeRTOS heap allocation...\r\n");
        void *test_ptr = pvPortMalloc(100);
        uart_puts("Test allocation (100 bytes): ");
        if (test_ptr) {
            uart_puts("SUCCESS at 0x");
            uart_hex((unsigned int)test_ptr);
            uart_puts("\r\n");
            // Note: heap_1 doesn't support freeing memory
            uart_puts("Note: Using heap_1 - memory cannot be freed\r\n");
        } else {
            uart_puts("FAILED\r\n");
        }
    } else {
        uart_puts("Basic memory access: FAILED - memory not writable\r\n");
    }
    
    uart_puts("Free heap size: ");
    uart_decimal(xPortGetFreeHeapSize());
    uart_puts(" bytes\r\n");
    uart_puts("Minimum ever free: ");
    uart_decimal(xPortGetMinimumEverFreeHeapSize());
    uart_puts(" bytes\r\n");
    
    uart_puts("Stack sizes requested:\r\n");
    uart_puts("  MemPattern: ");
    uart_decimal(configMINIMAL_STACK_SIZE * 4 * sizeof(StackType_t));
    uart_puts(" bytes\r\n");
    uart_puts("  PLC: ");
    uart_decimal(configMINIMAL_STACK_SIZE * 2 * sizeof(StackType_t));
    uart_puts(" bytes\r\n");
    uart_puts("  Demo: ");
    uart_decimal(configMINIMAL_STACK_SIZE * sizeof(StackType_t));
    uart_puts(" bytes\r\n");
    
    // Create multiple tasks like a real system
    uart_puts("=== CREATING MEMORY PATTERN TASK ===\r\n");
    uart_puts("About to call xTaskCreate for MemPattern...\r\n");
    BaseType_t result1 = xTaskCreate(vMemoryPatternTask, "MemPattern", configMINIMAL_STACK_SIZE * 4, NULL, 3, NULL);
    uart_puts("xTaskCreate returned!\r\n");
    uart_puts("Memory Pattern task creation result: ");
    uart_decimal(result1);
    if (result1 == pdPASS) {
        uart_puts(" (SUCCESS)\r\n");
    } else {
        uart_puts(" (FAILED - insufficient heap memory)\r\n");
    }
    uart_puts("Free heap after task 1: ");
    uart_decimal(xPortGetFreeHeapSize());
    uart_puts(" bytes\r\n");
    
    uart_puts("=== CREATING PLC TASK ===\r\n");
    BaseType_t result2 = xTaskCreate(vPLCMain, "PLC", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL);
    uart_puts("PLC task creation result: ");
    uart_decimal(result2);
    if (result2 == pdPASS) {
        uart_puts(" (SUCCESS)\r\n");
    } else {
        uart_puts(" (FAILED - insufficient heap memory)\r\n");
    }
    uart_puts("Free heap after task 2: ");
    uart_decimal(xPortGetFreeHeapSize());
    uart_puts(" bytes\r\n");
    
    uart_puts("=== CREATING DEMO TASK ===\r\n");
    BaseType_t result3 = xTaskCreate(vDemoTask, "Demo", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    uart_puts("Demo task creation result: ");
    uart_decimal(result3);
    if (result3 == pdPASS) {
        uart_puts(" (SUCCESS)\r\n");
    } else {
        uart_puts(" (FAILED - insufficient heap memory)\r\n");
    }
    uart_puts("Free heap after task 3: ");
    uart_decimal(xPortGetFreeHeapSize());
    uart_puts(" bytes\r\n");
    
    uart_puts("Starting FreeRTOS scheduler...\r\n");
    uart_puts("Tasks will begin running momentarily...\r\n");
    
    vTaskStartScheduler();
    
    // This should never be reached
    uart_puts("CRITICAL ERROR: Scheduler returned unexpectedly!\r\n");
    while (1) {
        delay(1000);
        uart_puts("System halted.\r\n");
    };
}

// Required idle hook function for FreeRTOS
void vApplicationIdleHook(void) {
    static uint32_t idle_counter = 0;
    
    // Simple heartbeat every ~1 million idle cycles
    if ((idle_counter & 0xFFFFF) == 0) {
        uart_puts(".");  // Minimal heartbeat
    }
    idle_counter++;
}

