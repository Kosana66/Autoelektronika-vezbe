
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
typedef struct taskInfo_ {
	SemaphoreHandle_t clk_in;
	SemaphoreHandle_t clk_out;
	unsigned char* disp;
} taskInfo;

/* Local function declaration */
static void first_task(void* pvParams);
static void TimerCallback(TimerHandle_t tmH);

/* Local function implementation */

static void TimerCallback(TimerHandle_t tmH)
{
	static unsigned char count = 0;
	static unsigned char secount = 0;

	select_7seg_digit(3 - count);
	set_7seg_digit(hexnum[dispMem[count]]);

	count++;
	count &= 0x03;

	secount++;
	if (secount == 25) //25*20ms=500ms
	{
		secount = 0;
		xSemaphoreGive(s1);
	}

	//za zadatak pod b)
	//select_7seg_digit(4);// selektovan 5 displej
	//set_7seg_digit(hexnum[uxTaskGetNumberOfTasks()]);		// ispisujemo broj taskova

}

static void first_task(void* pvParams)
{
	unsigned char cifra = 0;
	taskInfo* tinf_local;
	while (1)
	{
		tinf_local = (taskInfo*)pvParams;// castujemo iz pointera  void*  u pointer  taskInfo*

		xSemaphoreTake(tinf_local->clk_in, portMAX_DELAY);
		cifra++;

		if (cifra == 10) { // ako je broj dostigao vrednost 10, treba da se inkrementira vrednost vise cifre na displeju
			cifra = 0;
			xSemaphoreGive(tinf_local->clk_out);// dajemo takstni signal visoj cifri preko semafora 
		}

		*(tinf_local->disp) = cifra;// upisujemo cifru u odgovarajuci clan niza dispMem

		//za zadatak pod b)
		//ako je dostigao broj 90 i ako vec nije obrisan
		/*if (dispMem[1] == 9 && tC != NULL) {
			vTaskDelete(tC); 
			tC = NULL;
		}*/
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
	
	// SEMAPHORES
	// napomena: semafori moraju biti napravljeni pre kreiranja instance strukture taskInfo u kojoj se koriste
	s1 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s1 == NULL) while (1);

	s2 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s2 == NULL) while (1);

	s3 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s3 == NULL) while (1);

	s4 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s4 == NULL) while (1);
	
	taskInfo  tinf1 = {
		s1,
		s2,
		&dispMem[0]	// moglo je i :  dispMem
	};

	taskInfo  tinf2 = {
		s2,
		s3,
		&dispMem[1]	// moglo je i : dispMem+1
	};

	taskInfo  tinf3 = {
		s3,
		s4,
		&dispMem[2]	// moglo je i : dispMem+2
	};

	taskInfo  tinf4 = {
		s4,
		NULL,
		&dispMem[3]	// moglo je i : dispMem+3
	};

	// TASKS
	if (xTaskCreate(first_task, NULL, configMINIMAL_STACK_SIZE, &tinf1, 2, &tA) != pdPASS)
		while (1);  	// ID taska 0, referenca tA	
	if (xTaskCreate(first_task, NULL, configMINIMAL_STACK_SIZE, &tinf2, 2, &tB) != pdPASS)
		while (1);  	// ID taska 1, referenca tB
	if (xTaskCreate(first_task, NULL, configMINIMAL_STACK_SIZE, &tinf3, 2, &tC) != pdPASS)
		while (1);  	// ID taska 2, referenca tC
	if (xTaskCreate(first_task, NULL, configMINIMAL_STACK_SIZE, &tinf4, 2, &tD) != pdPASS)
		while (1);  	// ID taska 3, referenca tD



	// TIMERS
	
	myTimer1 = xTimerCreate(
		NULL,				/* tekstualni naziv tajmera, nije nepohodan, koristi se samo za debug  */
		pdMS_TO_TICKS(10),		/* perioda softverskog tajmera u "ticks" (konvertuju se ms). */
		pdTRUE,				/* xAutoReload kao parametar je podesen na pdTRUE, tako da je ovo ciklicni tajmer */
		NULL,				/* identifikacija ovog tajmera sa ID-jem*/
		TimerCallback);			/* ova funkcija se izvrsava kada se zavrsi jedna perioda tajmera */
	if (myTimer1 == NULL) { while (1); }    // provera kreiranja tajmera

	xTimerStart(myTimer1, 0);

	

	
	// START SCHEDULER
	vTaskStartScheduler();
}