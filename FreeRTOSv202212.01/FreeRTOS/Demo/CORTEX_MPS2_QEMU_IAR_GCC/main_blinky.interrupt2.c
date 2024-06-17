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

QueueHandle_t xIntegerQueue;
QueueHandle_t xStringQueue;

/* Function declarations */
static void vIntegerGenerator( void *pvParameters );
static void ulExampleInterruptHandler( void );
void vPortGenerateSimulatedInterrupt(uint32_t ulInterruptNumber);
void vPortSetInterruptHandler(uint32_t ulInterruptNumber, void (*pvHandler)(void));

/*-----------------------------------------------------------*/
static void vIntegerGenerator( void *pvParameters )
{
    TickType_t xLastExecutionTime;
    uint32_t ulValueToSend = 0;
    int i;

    /* Mark the parameter as unused */
    ( void ) pvParameters;

    /* Initialize the variable used by the call to vTaskDelayUntil(). */
    xLastExecutionTime = xTaskGetTickCount();

    for( ;; )
    {
        /* This is a periodic task. Block until it is time to run again. The task will execute every 200ms. */
        vTaskDelayUntil( &xLastExecutionTime, pdMS_TO_TICKS( 200 ) );

        /* Send five numbers to the queue */
        for( i = 0; i < 5; i++ )
        {
            xQueueSendToBack( xIntegerQueue, &ulValueToSend, 0 );
            ulValueToSend++;
        }

        printf( "Generator task - About to generate an interrupt.\r\n" );
        vPortGenerateSimulatedInterrupt( mainINTERRUPT_NUMBER );
        printf( "Generator task - Interrupt generated.\r\n\r\n\r\n" );
    }
}

void vPortGenerateSimulatedInterrupt(uint32_t ulInterruptNumber)
{
    /* Generate a simulated interrupt. */
    /* This might involve setting a bit in an interrupt register or similar in real system. */
    printf("Simulated interrupt generated for interrupt number: %lu\n", (unsigned long)ulInterruptNumber);
    ulExampleInterruptHandler();  // Directly call the interrupt handler for simulation
}

static void ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t ulReceivedNumber;

    /* The strings are declared static const -> not on the interrupt service routine's stack -> exist even when ISR not executing. */
    static const char *pcStrings[] =
    {
        "String 0\r\n",
        "String 1\r\n",
        "String 2\r\n",
        "String 3\r\n"
    };

    /* Read from the queue until the queue is empty. */
    while( xQueueReceiveFromISR( xIntegerQueue, &ulReceivedNumber, &xHigherPriorityTaskWoken ) != errQUEUE_EMPTY )
    {
        /* Truncate the received value to the last two bits (values 0 to 3 inclusive), then use the truncated value as an index into the pcStrings[] array to select a string (char *) to send on the other queue. */
        ulReceivedNumber &= 0x03;
        xQueueSendToBackFromISR( xStringQueue, &pcStrings[ ulReceivedNumber ], &xHigherPriorityTaskWoken );
    }

    /* If receiving from xIntegerQueue caused a task to leave the Blocked state, and if the priority of the task that left the Blocked state is higher than the priority of the task in the Running state, then xHigherPriorityTaskWoken will have been set to pdTRUE inside xQueueReceiveFromISR(). If sending to xStringQueue caused a task to leave the Blocked state, and if the priority of the task that left the Blocked state is higher than the priority of the task in the Running state, then xHigherPriorityTaskWoken will have been set to pdTRUE inside xQueueSendToBackFromISR(). xHigherPriorityTaskWoken is used as the parameter to portYIELD_FROM_ISR(). If xHigherPriorityTaskWoken equals pdTRUE then calling portYIELD_FROM_ISR() will request a context switch. If xHigherPriorityTaskWoken is still pdFALSE then calling portYIELD_FROM_ISR() will have no effect. The implementation of portYIELD_FROM_ISR() used by the Windows port includes a return statement, which is why this function does not explicitly return a value. */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void vStringPrinter( void *pvParameters )
{
    char *pcString;

    /* Mark the parameter as unused */
    ( void ) pvParameters;

    for( ;; )
    {
        /* Block on the queue to wait for data to arrive. */
        xQueueReceive( xStringQueue, &pcString, portMAX_DELAY );
        /* Print out the string received. */
        printf( "%s", pcString );
    }
}

void vApplicationIdleHook(void)
{
    /* This function is called on each cycle of the idle task. */
}

void main_blinky( void )
{
    xIntegerQueue = xQueueCreate( 10, sizeof( uint32_t ) );
    xStringQueue = xQueueCreate( 10, sizeof( char * ) );

    /* Create the task that uses a queue to pass integers to the interrupt service routine. The task is created at priority 1. */
    xTaskCreate( vIntegerGenerator, "IntGen", 1000, NULL, 1, NULL );
    /* Create the task that prints out the strings sent to it from the interrupt service routine. This task is created at the higher priority of 2. */
    xTaskCreate( vStringPrinter, "String", 1000, NULL, 2, NULL );
    /* Install the handler for the software interrupt. The syntax necessary to do this is dependent on the FreeRTOS port being used. The syntax shown here can only be used with the FreeRTOS Windows port, where such interrupts are only simulated. */
    vPortSetInterruptHandler( mainINTERRUPT_NUMBER, ulExampleInterruptHandler );
    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();  
    for( ;; );
}

void vPortSetInterruptHandler(uint32_t ulInterruptNumber, void (*pvHandler)(void))
{
    /* This is a dummy implementation to demonstrate the setting of an interrupt handler. */
    /* Real system would set the interrupt handler in the interrupt vector table. */
    printf("Interrupt handler set for interrupt number: %lu\n", (unsigned long)ulInterruptNumber);
}
