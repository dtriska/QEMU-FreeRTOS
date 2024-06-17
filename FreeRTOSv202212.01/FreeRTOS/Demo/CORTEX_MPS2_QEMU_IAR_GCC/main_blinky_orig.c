/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The times are converted from
milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 200UL )
#define mainTIMER_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 2000UL )

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH					( 2 )

/* The values sent to the queue receive task from the queue send task and the
queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK			( 100UL )
#define mainVALUE_SENT_FROM_TIMER			( 200UL )

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/*
 * The callback function executed when the software timer expires.
 */
static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle );

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/* A software timer that is started from the tick hook. */
static TimerHandle_t xTimer = NULL;

/* Simulated LED toggling function */
static void vToggleLED( void );

/*-----------------------------------------------------------*/

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void main_blinky( void )
{
    const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;

    /* Create the queue. */
    xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

    if( xQueue != NULL )
    {
        /* Start the two tasks as described in the comments at the top of this
        file. */
        xTaskCreate( prvQueueReceiveTask, "Rx", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );

        xTaskCreate( prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

        /* Create the software timer, but don't start it yet. */
        xTimer = xTimerCreate( "Timer", xTimerPeriod, pdTRUE, NULL, prvQueueSendTimerCallback );

        xTimerStart( xTimer, 0 ); /* The scheduler has not started so use a block time of 0. */

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
    }

    for( ;; );
}

/*-----------------------------------------------------------*/

static void prvQueueSendTask( void *pvParameters )
{
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK_SEND_FREQUENCY_MS;
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TASK;

    /* Prevent the compiler warning about the unused parameter. */
    ( void ) pvParameters;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for( ;; )
    {
        /* Place this task in the blocked state until it is time to run again.
        The block time is specified in ticks, pdMS_TO_TICKS() was used to
        convert a time specified in milliseconds into a time specified in ticks.
        While in the Blocked state this task will not consume any CPU time. */
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );

        /* Send to the queue - causing the queue receive task to unblock and
        write to the console.  0 is used as the block time so the send operation
        will not block - it shouldn't need to block as the queue should always
        have at least one space at this point in the code. */
        xQueueSend( xQueue, &ulValueToSend, 0U );

        /* Toggle LED to indicate message sent */
        vToggleLED();
    }
}

/*-----------------------------------------------------------*/

static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle )
{
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TIMER;

    /* Avoid compiler warnings resulting from the unused parameter. */
    ( void ) xTimerHandle;

    /* Send to the queue - causing the queue receive task to unblock and
    write out a message.  This function is called from the timer/daemon task, so
    must not block.  Hence the block time is set to 0. */
    xQueueSend( xQueue, &ulValueToSend, 0U );

    /* Toggle LED to indicate message sent */
    vToggleLED();
}

/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{
    uint32_t ulReceivedValue;

    /* Prevent the compiler warning about the unused parameter. */
    ( void ) pvParameters;

    for( ;; )
    {
        /* Wait until something arrives in the queue */
        xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

        /*  To get here something must have been received from the queue, but
        is it an expected value? */
        if( ulReceivedValue == mainVALUE_SENT_FROM_TASK )
        {
            /* It is normally not good to call printf() from an embedded system,
            although it is ok in this simulated case. */
            printf( "Message received from task\r\n" );
        }
        else if( ulReceivedValue == mainVALUE_SENT_FROM_TIMER )
        {
            printf( "Message received from software timer\r\n" );
        }
        else
        {
            printf( "Unexpected message\r\n" );
        }

        /* Toggle LED to indicate message received */
        vToggleLED();
    }
}

/*-----------------------------------------------------------*/

static void vToggleLED( void )
{
    /* Simulated LED toggling */
    static BaseType_t xLEDState = pdFALSE;

    /* Toggle the LED state */
    xLEDState = !xLEDState;

    /* Print LED state to console */
    printf("LED toggled %s\r\n", (xLEDState == pdTRUE) ? "ON" : "OFF");
}
/*-----------------------------------------------------------*/
