/* Standard includes. */
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "FreeRTOSConfig.h"

/* Priorities at which the tasks are created. */
#define mainTEST_1_PRIORITY      ( tskIDLE_PRIORITY + 2 )
#define mainTEST_2_PRIORITY      ( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue. The times are converted from
milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS   pdMS_TO_TICKS( 200UL )
#define mainTIMER_SEND_FREQUENCY_MS  pdMS_TO_TICKS( 2000UL )

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH             ( 2 )

/* The values sent to the queue receive task from the queue send task and the
queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK     ( 100UL )
#define mainVALUE_SENT_FROM_TIMER    ( 200UL )

/* Automatic Timer Period */
#define mainAUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 500 )

/* Static messages to output for each task */
static const char * pcTextForTest1 = "Test 1 is running";
static const char * pcTextForTest2 = "Test 2 is running";

/*-----------------------------------------------------------*/

/* Queue creation that will be used in Queue Send and Receive Tasks */
QueueHandle_t xQueue;

/* Define an enumerated type used to identify the source of the data. */
typedef enum
{
    eSender1,
    eSender2
} DataSource_t;

/* Define the structure type that will be passed on the queue. */
typedef struct
{
    int ucValue;
    DataSource_t eDataSource;
} Data_t;

/* Declare two variables of type Data_t that will be passed on the queue. */
static const Data_t xStructsToSend[ 2 ] =
{
    { 100, eSender1 }, /* Used by Sender1. */
    { 200, eSender2 } /* Used by Sender2. */
};

/* Declare a variable that will be incremented by the hook function. */
int ulIdleCycleCount = 0;

/* 
 * Idle hook functions MUST be called vApplicationIdleHook(), take no
 * parameters, and return void.
 */
void vApplicationIdleHook( void )
{
    /* This hook function does nothing but increment a counter. */
    ulIdleCycleCount++;
}

// void vTest1( void *pvParameters )
// {
//     const char *pcTaskName = (const char *) pvParameters;

//     /* Block for 500ms. */
//     const TickType_t xDelay = pdMS_TO_TICKS( 500 );

//     for( ;; )
//     {
//         printf( "%s\n", pcTaskName );
//         printf( "%d\n", ulIdleCycleCount );

//         vTaskDelay( xDelay );
//     }
// }

// void vTest2( void *pvParameters )
// {
//     const char *pcTaskName = (const char *) pvParameters;

//     /* Block for 200ms. */
//     const TickType_t xDelay = pdMS_TO_TICKS( 200 );

//     for( ;; )
//     {
//         printf( "%s\n", pcTaskName );

//         vTaskDelay( xDelay );
//     }
// } 

static void vSenderTask(void *pvParameters)
{
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 100 );

    for(;;)
    {
        xStatus = xQueueSendToBack(xQueue, pvParameters, xTicksToWait); 

        if( xStatus != pdPASS )
        {
            printf( "Could not send to the queue.\n" );
        }
    }
}

static void vReceiverTask(void *pvParameters)
{
    Data_t xReceivedStructure;
    BaseType_t xStatus;

    for(;;)
    {
        /* This call should always find the queue empty because this task will
        immediately remove any data that is written to the queue. */
        if( uxQueueMessagesWaiting( xQueue ) != 3 )
        {
            printf( "Queue should always be full based on prio!\n" );
        }

        xStatus = xQueueReceive( xQueue, &xReceivedStructure, 0 ); 

        if( xStatus == pdPASS )
        {
            /* Data was successfully received from the queue, print out the
            received value and the source of the value. */
            if( xReceivedStructure.eDataSource == eSender1 )
            {
                printf( "From Sender 1 = %d\n", xReceivedStructure.ucValue );
            }
            else
            {
                printf( "From Sender 2 = %d\n", xReceivedStructure.ucValue );
            }
        }
        else
        {
            /* Data was not received from the queue even after waiting for */
            printf( "Could not receive from the queue.\n" );
        }
    }
}

static void prvAutoReloadTimerCallback( TimerHandle_t xTimer )
{
    TickType_t xTimeNow;
    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();
    /* Output a string to show the time at which the callback was executed. */
    printf( "Auto-reload timer callback executing");
}

void main_blinky( void )
{
    /* xTaskCreate( vTest1, "Test 1", 1000, (void *)pcTextForTest1, mainTEST_1_PRIORITY, NULL );
    xTaskCreate( vTest2, "Test 2", 1000, (void *)pcTextForTest2, mainTEST_2_PRIORITY, NULL ); */

    TimerHandle_t xAutoReloadTimer;
    BaseType_t xTimer1Started;

    /* Create the auto-reload timer, storing the handle to the created timer
    in xAutoReloadTimer. */
    /* Setting uxAutoRealod to pdTRUE creates an auto-reload timer. */
    /* Callback function to be used by the software timer being created. */
    xAutoReloadTimer = xTimerCreate("AutoReload", mainAUTO_RELOAD_TIMER_PERIOD, pdTRUE, 0, prvAutoReloadTimerCallback );

    /* The queue is created to hold a maximum of 3 structures of type Data_t. */
    xQueue = xQueueCreate( 3, sizeof( Data_t ) );

    if( xQueue != NULL )
    {
        if ( xAutoReloadTimer != NULL )
        {
            xTimer1Started = xTimerStart( xAutoReloadTimer, 0 );
        }
        xTaskCreate( vSenderTask, "Sender1", 1000, &( xStructsToSend[ 0 ] ), 2, NULL );
        xTaskCreate( vSenderTask, "Sender2", 1000, &( xStructsToSend[ 1 ] ), 2, NULL );
        
        xTaskCreate( vReceiverTask, "Receiver", 1000, NULL, 1, NULL );
        
        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }
    for( ;; );
}
