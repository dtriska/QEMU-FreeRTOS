/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"

/* Mock hardware specific includes. */
#include "mock_uart.h"
#include "mock_gpio.h"

/* Priorities at which the tasks are created. */
#define mainBLINKY_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1 )
#define mainUART_TASK_PRIORITY        ( tskIDLE_PRIORITY + 2 )

/* The rate at which the LED blinks. */
#define mainBLINKY_DELAY_MS           pdMS_TO_TICKS( 500UL )

/* UART buffer size. */
#define mainUART_BUFFER_SIZE          ( 128 )

/* Function declarations */
static void vBlinkyTask(void *pvParameters);
static void vUARTTask(void *pvParameters);
static void vUARTInterruptHandler(void);

/* UART handle */
static SemaphoreHandle_t xUARTSemaphore = NULL;

/* Flag to control blinking */
static volatile int xBlinkingEnabled = 1;

/*-----------------------------------------------------------*/

/* Blinky task to blink an LED */
static void vBlinkyTask(void *pvParameters)
{
    for (;;)
    {
        if (xBlinkingEnabled)
        {
            /* Toggle the LED */
            gpio_toggle(GPIO_LED1);
        }
        
        /* Delay for a period */
        vTaskDelay(mainBLINKY_DELAY_MS);
    }
}

/* UART task to handle ground system communication */
static void vUARTTask(void *pvParameters)
{
    char buffer[mainUART_BUFFER_SIZE];
    int len;

    for (;;)
    {
        /* Wait for UART data */
        if (xSemaphoreTake(xUARTSemaphore, portMAX_DELAY) == pdTRUE)
        {
            /* Read the received data */
            len = uart_read(buffer, mainUART_BUFFER_SIZE);

            /* Echo the received data back */
            uart_write(buffer, len);

            /* Process the received command (if any) */
            if (strncmp(buffer, "BLINK ON", len) == 0)
            {
                xBlinkingEnabled = 1;
            }
            else if (strncmp(buffer, "BLINK OFF", len) == 0)
            {
                xBlinkingEnabled = 0;
            }
        }
    }
}

/* UART interrupt handler */
void vUARTInterruptHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Give the semaphore to unblock the UART task */
    xSemaphoreGiveFromISR(xUARTSemaphore, &xHigherPriorityTaskWoken);

    /* Perform a context switch if necessary */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*-----------------------------------------------------------*/

void main_blinky(void)
{
    /* Initialize GPIO for LED */
    gpio_init(GPIO_LED1);

    /* Initialize UART */
    uart_init();
    uart_set_interrupt_handler(vUARTInterruptHandler);

    /* Create the semaphore for UART handling */
    xUARTSemaphore = xSemaphoreCreateBinary();

    /* Create the blinky task */
    xTaskCreate(vBlinkyTask, "Blinky", configMINIMAL_STACK_SIZE, NULL, mainBLINKY_TASK_PRIORITY, NULL);

    /* Create the UART task */
    xTaskCreate(vUARTTask, "UART", configMINIMAL_STACK_SIZE, NULL, mainUART_TASK_PRIORITY, NULL);

    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    /* Infinite loop */
    for (;;);
}

/*-----------------------------------------------------------*/
/* Mock implementations of UART and GPIO functions */

/* GPIO implementation */
void gpio_init(int pin)
{
    printf("GPIO %d initialized.\n", pin);
}

void gpio_toggle(int pin)
{
    static int state = 0;
    state = !state;
    printf("GPIO %d toggled to %d.\n", pin, state);
}

/* UART implementation */
void uart_init(void)
{
    printf("UART initialized.\n");
}

void uart_set_interrupt_handler(void (*handler)(void))
{
    printf("UART interrupt handler set.\n");
}

int uart_read(char *buffer, int length)
{
    /* Simulate receiving "BLINK ON" or "BLINK OFF" command from ground station */
    strcpy(buffer, "BLINK ON");
    return strlen(buffer);
}

void uart_write(const char *buffer, int length)
{
    printf("UART write: %.*s\n", length, buffer);
}

/*-----------------------------------------------------------*/

/* Idle hook function implementation */
void vApplicationIdleHook(void)
{
    /* This function is called on each cycle of the idle task. */
    /* You can add any idle time processing here if needed. */
}
