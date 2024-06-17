/* Standard includes. */
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"  /* Include for semaphore functions */
#include "FreeRTOSConfig.h"
#include "portmacro.h"  /* Include for port layer functions */

/* Priorities at which the tasks are created. */
#define mainTEST_1_PRIORITY      ( tskIDLE_PRIORITY + 2 )
#define mainTEST_2_PRIORITY      ( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue. The times are converted from
milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS   pdMS_TO_TICKS( 200UL )
#define mainTIMER_SEND_FREQUENCY_MS  pdMS_TO_TICKS( 2000UL )

/* The values sent to the queue receive task from the queue send task and the
queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK     ( 100UL )
#define mainVALUE_SENT_FROM_TIMER    ( 200UL )

#define mainINTERRUPT_NUMBER 40

/* Declare the semaphore handle */
static SemaphoreHandle_t xBinarySemaphore = NULL;

/*-----------------------------------------------------------*/

static void vPeriodicTask( void *pvParameters )
{
    const TickType_t xDelay500ms = pdMS_TO_TICKS( 500UL );
    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* Block until it is time to generate the software interrupt again. */
        vTaskDelay( xDelay500ms );
        printf( "Periodic task - About to generate an interrupt.\n" );
        vPortGenerateSimulatedInterrupt( mainINTERRUPT_NUMBER );
        printf( "Periodic task - Interrupt generated.\n" );
    }   
}

static void vHandlerTask( void *pvParameters )
{
    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        /* To get here the event must have occurred. Process the event (in this case, just print out a message). */
        printf( "Handler task - Processing event.\n" );
    }
}

/* The interrupt handler should be void type as it does not return a value. */
static void ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    printf( "Interrupt handler - Giving semaphore.\n" );
    xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void vPortGenerateSimulatedInterrupt(uint32_t ulInterruptNumber)
{
    /* Generate a simulated interrupt. */
    /* This might involve setting a bit in an interrupt register or similar in real system. */
    printf("Simulated interrupt generated for interrupt number: %u\n", ulInterruptNumber);
    ulExampleInterruptHandler();  // Directly call the interrupt handler for simulation
}

void vPortSetInterruptHandler(uint32_t ulInterruptNumber, void (*pvHandler)(void))
{
    /* Need implementation to set an interrupt handler. */
    /* This might involve setting a vector table entry or similar. */
    printf("Setting interrupt handler for interrupt number: %u\n", ulInterruptNumber);
    /* This is a simplified placeholder. Typically, you'd set the vector table here. */
}

void vApplicationIdleHook(void)
{
    /* This function is called on each cycle of the idle task. */
}

void main_blinky( void )
{
    /* Create the binary semaphore */
    xBinarySemaphore = xSemaphoreCreateBinary();
    
    /* Check the semaphore was created successfully. */
    if( xBinarySemaphore != NULL )
    {
        /* Handle interrupt processing */
        xTaskCreate( vHandlerTask, "Handler", 1000, NULL, 3, NULL );

        /* Task to create software Interrupt */
        xTaskCreate( vPeriodicTask, "Periodic", 1000, NULL, 1, NULL );

        /* Set the interrupt handler */
        vPortSetInterruptHandler( mainINTERRUPT_NUMBER, ulExampleInterruptHandler );

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }
    
    for( ;; );
}
