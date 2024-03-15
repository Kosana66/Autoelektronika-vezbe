
// KERNEL INCLUDES
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

// HARDWARE SIMULATOR UTILITY FUNCTIONS  
#include "HW_access.h"

// TASK PRIORITIES 
#define task_prioritet		( tskIDLE_PRIORITY + 2 )

// 7-SEG NUMBER DATABASE - ALL HEX DIGITS [ 0 1 2 3 4 5 6 7 8 9 A B C D E F ]
static const char hexnum[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };

static BaseType_t myTask;
static SemaphoreHandle_t binSem1;
static TimerHandle_t myTimer1;

/* Local function declaration */
static void DisplayDigit_0();
static void first_task(void* pvParams);
static void TimerCallback(TimerHandle_t tmH);

/* Local function implementation */
static void DisplayDigit_0() {
	static uint8_t displayValue = 0;
	select_7seg_digit(0);
	if (displayValue == 0) {
		set_7seg_digit(hexnum[displayValue]);
		displayValue = 1;
	}
	else {
		set_7seg_digit(hexnum[displayValue]);
		displayValue = 0;
	}
}

static void TimerCallback(TimerHandle_t tmH)
{
	xSemaphoreGive(binSem1);
}

static void first_task(void* pvParams)
{
	xTimerStart(myTimer1, portMAX_DELAY); 
	while (1) {
		xSemaphoreTake(binSem1, portMAX_DELAY);
		DisplayDigit_0();
	}
}

// MAIN - SYSTEM STARTUP POINT 
void main_demo(void) {
	// INITIALIZATION OF THE PERIPHERALS
	init_7seg_comm();

	myTask = xTaskCreate(first_task,	/* funkcija koja implementira task */
		NULL, 				/* tekstualni naziv taska, nije neophodan, koristi se samo za debug */
		configMINIMAL_STACK_SIZE, 	/* velicina steka u bajtovima za ovaj task  */
		NULL, 				/* parametar koji se prosledjuje tasku, ovde se ne koristi*/
		task_prioritet,                 /* prioritet ovog taska, sto veci broj veci prioritet*/
		NULL);				/* referenca (handle) ovog taska, ovde se ne koristi */
	if (myTask != pdPASS) { while (1); }    // provera kreiranja taska

	// TIMERS
	myTimer1= xTimerCreate(
		NULL,				/* tekstualni naziv tajmera, nije nepohodan, koristi se samo za debug  */
		pdMS_TO_TICKS(500),		/* perioda softverskog tajmera u "ticks" (konvertuju se ms). */
		pdTRUE,				/* xAutoReload kao parametar je podesen na pdTRUE, tako da je ovo ciklicni tajmer */
		NULL,				/* identifikacija ovog tajmera sa ID-jem*/
		TimerCallback);			/* ova funkcija se izvrsava kada se zavrsi jedna perioda tajmera */
	if (myTimer1 == NULL) { while (1); }    // provera kreiranja tajmera
	
	// SEMAPHORES
	binSem1 = xSemaphoreCreateBinary();
	if (binSem1 == NULL) while (1); 	// provera kreiranja semafora

	// START SCHEDULER
	vTaskStartScheduler();
}

