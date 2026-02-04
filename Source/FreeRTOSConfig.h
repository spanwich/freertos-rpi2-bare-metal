/*
 * FreeRTOS Kernel V10.x.x
 * Configuration for Raspberry Pi 2B v1.2 (BCM2837 - Cortex-A53 @ 900MHz)
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* Hardware configuration */
#define configCPU_CLOCK_HZ                              ( ( unsigned long ) 900000000 ) /* 900 MHz */
#define configTICK_RATE_HZ                              ( ( TickType_t ) 1000 )         /* 1ms tick */
#define configPERIPH_BASE_ADDRESS                       0x3F000000
#define configUART_BASE                                 0x3F201000  /* PL011 UART0 */
#define configTIMER_BASE                                0x3F003000  /* System timer */

/* BCM2836/2837 ARM local interrupt controller (not standard GIC)
 * WARNING: BCM2837 does NOT have ARM GIC. It uses custom QA7 controller.
 * These values are set to satisfy FreeRTOS ARM_CA9 port requirements,
 * but interrupt handling must be customized for BCM2837.
 */
#define configINTERRUPT_CONTROLLER_BASE_ADDRESS         0x40000000  /* ARM local peripherals (QA7) */
#define configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET 0x0000      /* Direct access, no offset */

/* BCM2837 VideoCore interrupt controller for peripherals (GPIO, UART, Timer) */
#define configVC_IRQ_BASE_ADDRESS                       0x3F00B000  /* VideoCore IRQ controller */

/* Timer configuration - use ARM generic timer */
#define configSETUP_TICK_INTERRUPT()                    vConfigureTickInterrupt()
#define configCLEAR_TICK_INTERRUPT()                    vClearTickInterrupt()

/* BCM2837 GIC stub support - extern declarations */
extern volatile uint32_t bcm2837_stub_gic_pmr;
extern volatile uint32_t bcm2837_stub_gic_bpr;
extern volatile uint8_t bcm2837_stub_gic_priority[1024];

/* Redefine interrupt controller base to point to stub registers */
#undef configINTERRUPT_CONTROLLER_BASE_ADDRESS
#define configINTERRUPT_CONTROLLER_BASE_ADDRESS         ((uint32_t)&bcm2837_stub_gic_priority[0])

/* Function prototypes */
void vConfigureTickInterrupt(void);
void vClearTickInterrupt(void);

/* Scheduler configuration */
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configMAX_PRIORITIES                    ( 8 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 512 )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3

/* Memory allocation configuration */
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 32 * 1024 * 1024 ) ) /* 32MB */
#define configAPPLICATION_ALLOCATED_HEAP        0

/* Hook function configuration */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0
#define configCHECK_FOR_STACK_OVERFLOW          2

/* Run time and task stats gathering */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/* Co-routine configuration */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/* Software timer configuration */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

/* Queue and semaphore configuration */
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1

/* Optional functions */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetHandle                  1

/* ARM Cortex-A specific settings */
#define configUNIQUE_INTERRUPT_PRIORITIES       32
#define configMAX_API_CALL_INTERRUPT_PRIORITY   18  /* Higher priority number = lower priority */

/* Assertion configuration */
extern void vAssertCalled( unsigned long ulLine, const char * const pcFileName );
/* BCM2837-specific: Assertions enabled with GIC stub support */
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __LINE__, __FILE__ )

/* Definitions that map the FreeRTOS port interrupt handlers */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
