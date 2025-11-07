/*************************************************************************
 * Programa de prueba basado para LM3S6965.
 *
 * Basado en el demo provisto por FreeRTOS, para ejecutar sobre QEMU.
 *
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/
#include <stdlib.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_uart.h"
#include "sysctl.h"
#include "gpio.h"
#include "grlib.h"
#include "osram128x64x4.h"
#include "uart.h"
#include "bitmap.h"

/*-----------------------------------------------------------*/

/* Dimensions the buffer for text messages. */
#define mainMAX_MSG_LEN                     25

/* Constants used when writing strings to the display. */
#define mainCHARACTER_HEIGHT                ( 9 )
#define mainMAX_ROWS_128                    ( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96                     ( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64                     ( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE                      ( 15 )
#define ulSSI_FREQUENCY                     ( 3500000UL )

struct myTask {
	TickType_t wcet;
	TickType_t period;
};

struct myTask s1[] = {
	{1000, 4000},
	{1000, 5000},
	{2000, 8000}
};

struct myTask s2[] = {
	{1000, 4000},
	{1000, 8000},
	{2000, 9000}
};

struct myTask s3[] = {
	{1000, 4000},
	{1000, 6000},
	{1000, 8000},
	{3000, 13000}
};

struct myTask s4[] = {
	{1000, 4000},
	{2000, 7000},
	{1000, 12000},
	{2000, 14000}
};

struct myTask s5[] = {
	{1000, 5000},
	{1000, 9000},
	{1000, 10000},
	{2000, 15000},
	{2000, 16000}
};

struct myTask s6[] = {
	{1000, 6000},
	{1000, 8000},
	{2000, 11000},
	{1000, 15000},
	{2000, 17000}
};

struct myTask s7[] = {
	{1000, 4000},
	{1000, 7000},
	{2000, 10000},
	{2000, 14000}
};

struct myTask s8[] = {
	{1000, 4000},
	{1000, 8000},
	{2000, 10000},
	{2000, 14000}
};

struct myTask s9[] = {
	{1000, 5000},
	{1000, 8000},
	{2000, 12000},
	{1000, 15000},
	{1000, 16000}
};

struct myTask s10[] = {
	{1000, 5000},
	{1000, 8000},
	{1000, 12000},
	{1000, 13000},
	{2000, 16000}
};
// son las filas totales de la matriz
#define CANT_SYSTEMS_TP2	10
// cada fila
#define S1	0
#define S2	1
#define S3	2
#define S4	3
#define S5	4
#define S6	5
#define S7	6
#define S8	7
#define S9	8
#define S10	9
// la columna 0 (primera)
#define CANT_TASKS_SYSTEM	0
// la columna 1 (tarea)
#define SYSTEM_TASK	1

// matriz donde las filas son pares
// (int:cantidad de tareas, myTask[]:el sistema)
// es decir que cada fila tiene informacion de un sistema
void* tp2[CANT_SYSTEMS_TP2][2] = {
	{( void* )3, ( void* )s1},
	{( void* )3, ( void* )s2},
	{( void* )4, ( void* )s3},
	{( void* )4, ( void* )s4},
	{( void* )5, ( void* )s5},
	{( void* )5, ( void* )s6},
	{( void* )4, ( void* )s7},
	{( void* )4, ( void* )s8},
	{( void* )5, ( void* )s9},
	{( void* )5, ( void* )s10}
};

/* Tasks periods. */
#define TASK1_PERIOD 	4000
#define TASK2_PERIOD 	5000
#define TASK3_PERIOD 	8000

/* Tasks WCETs. */
#define TASK1_WCET		1000
#define TASK2_WCET		1000
#define TASK3_WCET		2000

/*-----------------------------------------------------------*/

/*
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );

/*
 * Basic polling UART write function.
 */
static void prvPrintString( const char * pcString );

/*
 * Busy wait the specified number of ticks.
 */
static void vBusyWait( TickType_t ticks );

/*
 * Periodic task.
 */
static void prvTask( void* pvParameters );

/*-----------------------------------------------------------*/

/* Functions to access the OLED.  The one used depends on the dev kit
being used. */
void ( *vOLEDInit )( uint32_t ) = NULL;
void ( *vOLEDStringDraw )( const char *, uint32_t, uint32_t, unsigned char ) = NULL;
void ( *vOLEDImageDraw )( const unsigned char *, uint32_t, uint32_t, uint32_t, uint32_t ) = NULL;
void ( *vOLEDClear )( void ) = NULL;

/*-----------------------------------------------------------*/

struct xTaskStruct {
	TickType_t wcet;
	TickType_t period;
};

typedef struct xTaskStruct xTask;

xTask task1 = { TASK1_WCET, TASK1_PERIOD };
xTask task2 = { TASK2_WCET, TASK2_PERIOD };
xTask task3 = { TASK3_WCET, TASK3_PERIOD };

/*************************************************************************
 * Main
 *************************************************************************/
int main( void )
{
	/* Initialise the trace recorder. */
	vTraceEnable( TRC_INIT );

    prvSetupHardware();

    /* Map the OLED access functions to the driver functions that are appropriate
    for the evaluation kit being used. */
    //configASSERT( ( HWREG( SYSCTL_DID1 ) & SYSCTL_DID1_PRTNO_MASK ) == SYSCTL_DID1_PRTNO_6965 );
    vOLEDInit = OSRAM128x64x4Init;
    vOLEDStringDraw = OSRAM128x64x4StringDraw;
    vOLEDImageDraw = OSRAM128x64x4ImageDraw;
    vOLEDClear = OSRAM128x64x4Clear;

    /* Initialise the OLED and display a startup message. */
    vOLEDInit( ulSSI_FREQUENCY );

    /* Print Hello World! to the OLED display. */
    static char cMessage[ mainMAX_MSG_LEN ];
    //sprintf(cMessage, "TP3 -->");
    vOLEDStringDraw( cMessage, 0, 0, mainFULL_SCALE );

    /* Print "Start!" to the UART. */
    prvPrintString("Start!\n\r");
    //char cMsg[ mainMAX_MSG_LEN ];
    //struct myTask systemTP2[];
    //systemTP2 = s1;
    //struct myTask *mySystem = &s1[0];


    int system_selected = S10;
	int N = (int) tp2[system_selected][CANT_TASKS_SYSTEM];
	//int N = sizeof(s1) / sizeof(s1[0]);
    for(int i=0; i<N ;i++){
    	//prvPrintString("pase...\n\r");
    	char prefix[] = "T";
    	char taskName[10];
    	int taskID = i + 1;
    	snprintf(taskName, sizeof(taskName), "%s%d", prefix, taskID);
    	//xTask *task = (xTask *)malloc(sizeof(xTask));
    	//task->wcet = s1[i].wcet;
    	//task->period = s1[i].period;
    	//sprintf( cMsg, "INFO: N -> %d\n\r", N);
    	xTaskCreate( prvTask, taskName, configMINIMAL_STACK_SIZE + 50, (void*) &(((struct myTask*) tp2[system_selected][SYSTEM_TASK])[i]), configMAX_PRIORITIES - taskID, NULL );
    	//xTaskCreate( prvTask, taskName, configMINIMAL_STACK_SIZE + 50, (void*) &s1[i], configMAX_PRIORITIES - taskID, NULL );
    }

    /* Creates the periodic tasks. *//*
    // funcion, nombre, pila (140 palabras), puntero a la tarea, prioridad (0 la mas baja), puntero para referenciar la tarea
    xTaskCreate( prvTask, "T1", configMINIMAL_STACK_SIZE + 50, (void*) &task1, configMAX_PRIORITIES - 1, NULL );
    xTaskCreate( prvTask, "T2", configMINIMAL_STACK_SIZE + 50, (void*) &task2, configMAX_PRIORITIES - 2, NULL );
    xTaskCreate( prvTask, "T3", configMINIMAL_STACK_SIZE + 50, (void*) &task3, configMAX_PRIORITIES - 3, NULL );
*/
    vTraceEnable( TRC_START );

    /* Launch the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
    for( ;; );
}
/*-----------------------------------------------------------*/

void prvSetupHardware( void )
{
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    a workaround to allow the PLL to operate reliably. */
    if( DEVICE_IS_REVA2 )
    {
        SysCtlLDOSet( SYSCTL_LDO_2_75V );
    }

    /* Set the clocking to run from the PLL at 50 MHz */
    SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

    /* Initialise the UART - QEMU usage does not seem to require this
    initialisation. */
    SysCtlPeripheralEnable( SYSCTL_PERIPH_UART0 );
    UARTEnable( UART0_BASE );
}
/*-----------------------------------------------------------*/

static void prvPrintString( const char * pcString )
{
    while( *pcString != 0x00 )
    {
        UARTCharPut( UART0_BASE, *pcString );
        pcString++;
    }
}
/*-----------------------------------------------------------*/

static void vBusyWait( TickType_t ticks )
{
    TickType_t elapsedTicks = 0;
    TickType_t currentTick = 0;
    while ( elapsedTicks < ticks ) {
        currentTick = xTaskGetTickCount();
        while ( currentTick == xTaskGetTickCount() ) {
            asm("nop");
        }
        elapsedTicks++;
    }
}
/*-----------------------------------------------------------*/

void prvTask( void *pvParameters )
{
	char cMessage[ mainMAX_MSG_LEN ];
	unsigned int uxReleaseCount = 0;
	TickType_t pxPreviousWakeTime = 0;
	xTask *task = (xTask*) pvParameters;

	for( ;; )
	{
		TickType_t startTick = xTaskGetTickCount();
        sprintf( cMessage, "S %s - %u - %u\n\r", pcTaskGetTaskName( NULL ), uxReleaseCount, startTick );

        prvPrintString( cMessage );

        vBusyWait( task->wcet - 50 );

		TickType_t endTick = xTaskGetTickCount();
		sprintf( cMessage, "E %s - %u - %u\n\r", pcTaskGetTaskName( NULL ), uxReleaseCount, endTick );

		prvPrintString( cMessage );

		vTaskDelayUntil( &pxPreviousWakeTime, task->period );

		uxReleaseCount += 1;
	}

	vTaskDelete( NULL );
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char *pcFile, uint32_t ulLine )
{
    volatile uint32_t ulSetTo1InDebuggerToExit = 0;
    {
        while( ulSetTo1InDebuggerToExit == 0 )
        {
            /* Nothing to do here.  Set the loop variable to a non zero value in
            the debugger to step out of this function to the point that caused
            the assertion. */
            ( void ) pcFile;
            ( void ) ulLine;
        }
    }
}

char* _sbrk_r (struct _reent *r, int incr)
{
    /* Just to keep the linker quiet. */
    ( void ) r;
    ( void ) incr;

    /* Check this function is never called by forcing an assert() if it is. */
    //configASSERT( incr == -1 );

    return NULL;
}

int __error__(char *pcFilename, unsigned long ulLine) {
    return 0;
}

