
// KERNEL INCLUDES
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

// HARDWARE SIMULATOR UTILITY FUNCTIONS  
#include "HW_access.h"

#define MAX_SEM_COUNT 10

// TASK PRIORITIES 
#define task_prioritet		( tskIDLE_PRIORITY + 2 )

// 7-SEG NUMBER DATABASE - ALL HEX DIGITS [ 0 1 2 3 4 5 6 7 8 9 A B C D E F ]
static const char hexnum[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };

static TimerHandle_t myTimer1;
static TaskHandle_t tA, tB, tC, tD;
static SemaphoreHandle_t s1, s2, s3, s4;

static unsigned char dispMem[4];

/* Local function declaration */
static void first_task(void* pvParams);
static void TimerCallback(TimerHandle_t tmH);

/* Local function implementation */



static void TimerCallback(TimerHandle_t tmH)
{
	static unsigned char count = 0;
	static unsigned char secount = 0;

	select_7seg_digit(3-count);
	set_7seg_digit(hexnum[dispMem[count]]);

	if (count < 3) 
		count++; 
	else 
		count = 0;

	secount++;
	if (secount == 25) //25*20ms=500ms
	{ 
		secount = 0;
		xSemaphoreGive(s1);
	}
}

static void first_task(void* pvParams)
{
	int s1_value = 0;
	unsigned char number = 0;
	uint32_t task_ID;
	while (1) 
	{
		number++;
		task_ID = (uint32_t)pvParams;
		if (task_ID == 0)
		{
			xSemaphoreTake(s1, portMAX_DELAY);
			if (number == 10) // kad se dostigne vrednost 10, onda se poveca sledeca cifra displeja
			{ 
				number = 0;
				xSemaphoreGive(s2);	// dajemo taktni signal visoj cifri preko semafora 
			}
		}
		if (task_ID == 1)
		{
			xSemaphoreTake(s2, portMAX_DELAY);
			if (number == 10) 
			{
				number = 0;
				xSemaphoreGive(s3);
			}
		}
		if (task_ID == 2)
		{
			xSemaphoreTake(s3, portMAX_DELAY);
			if (number == 10) 
			{			
				number = 0;
				xSemaphoreGive(s4);
			}
		}
		if (task_ID == 3)
		{
			xSemaphoreTake(s4, portMAX_DELAY);
			if (number == 10) 
			{	
				number = 0;
				//resenja zadatka pod b
				//vTaskDelete(tA); //brisemo task sa ID 0 preko njegovre reference tA 
				//vTaskDelete(tB); //brisemo task sa ID 1 preko njegovre reference tB 
				//vTaskDelete(tC); //brisemo task sa ID 2 preko njegovre reference tC 
				//vTaskDelete(NULL); //brisemo task sa ID 3 preko NULL jer je to aktivan task
			}
		}
		dispMem[task_ID] = number; // upisujemo vrednost broja u niz cija vrednost se prikazuje na displeju
		select_7seg_digit(4);
		if (s1_value < uxSemaphoreGetCount(s1)) s1_value = uxSemaphoreGetCount(s1);
			set_7seg_digit(hexnum[s1_value]);
			
	}
}

// MAIN - SYSTEM STARTUP POINT 
void main_demo(void) {
	// INITIALIZATION OF THE PERIPHERALS
	init_7seg_comm();
	
	// pocetno stanje displeja je 0000
	dispMem[0] = 0;
	dispMem[1] = 0;
	dispMem[2] = 0;
	dispMem[3] = 0;

	// TASKS
	if (xTaskCreate(first_task, NULL, configMINIMAL_STACK_SIZE, (void*)0, 2, &tA) != pdPASS) 
		while (1);  	// ID taska 0, referenca tA	
	if (xTaskCreate(first_task, NULL, configMINIMAL_STACK_SIZE, (void*)1, 2, &tB) != pdPASS) 
		while (1);  	// ID taska 1, referenca tB
	if (xTaskCreate(first_task, NULL, configMINIMAL_STACK_SIZE, (void*)2, 2, &tC) != pdPASS) 
		while (1);  	// ID taska 2, referenca tC
	if (xTaskCreate(first_task, NULL, configMINIMAL_STACK_SIZE, (void*)3, 2, &tD) != pdPASS) 
		while (1);  	// ID taska 3, referenca tD
	

	
	// TIMERS
	myTimer1= xTimerCreate(
		NULL,				/* tekstualni naziv tajmera, nije nepohodan, koristi se samo za debug  */
		pdMS_TO_TICKS(20),		/* perioda softverskog tajmera u "ticks" (konvertuju se ms). */
		pdTRUE,				/* xAutoReload kao parametar je podesen na pdTRUE, tako da je ovo ciklicni tajmer */
		NULL,				/* identifikacija ovog tajmera sa ID-jem*/
		TimerCallback);			/* ova funkcija se izvrsava kada se zavrsi jedna perioda tajmera */
	if (myTimer1 == NULL) { while (1); }    // provera kreiranja tajmera
	
	xTimerStart(myTimer1, 0);
	
	// SEMAPHORES
	s1 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s1 == NULL) while (1);

	s2 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s2 == NULL) while (1);

	s3 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s3 == NULL) while (1);

	s4 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s4 == NULL) while (1);


	// START SCHEDULER
	vTaskStartScheduler();
}