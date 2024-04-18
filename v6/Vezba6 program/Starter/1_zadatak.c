
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

static SemaphoreHandle_t binSem1;
static TaskHandle_t tA, tB;

#define mainVALUE_SENT_FROM_TASK			( 100UL )
#define mainVALUE_SENT_FROM_TIMER			( 200UL )

static QueueHandle_t myQueue = NULL;

TimerHandle_t tH;

/* Local function declaration */
static void TimerCallback(TimerHandle_t tmH);
static void QueueSend_tsk(void* pvParams);
static void QueueReceive_tsk(void* pvParameters);


/* Local function implementation */
static void TimerCallback(TimerHandle_t tmH)
{
	uint32_t ulValueToSend = mainVALUE_SENT_FROM_TIMER;
	xQueueSend(myQueue, &ulValueToSend, 0U);// salje podatak iz tajmera
}

static void QueueSend_tsk(void* pvParams)
{
	uint32_t ulValueToSend = mainVALUE_SENT_FROM_TASK;
	
	for (;; ) // beskonacna petlja
	{
		vTaskDelay(pdMS_TO_TICKS(400UL)); // task je blokiran 400 ms
		xQueueSend(myQueue, &ulValueToSend, 0U);// salje podatak iz taska u red myQueue
	}

}
static void QueueReceive_tsk(void* pvParameters)
{
	uint32_t ulReceivedValue;
	uint16_t a_num = 0;
	uint16_t b_num = 0;
	for (;; )
	{
		xQueueReceive(myQueue, &ulReceivedValue, portMAX_DELAY); // task ceka u blokiranom stanju dok ne dobije podatak iz Queue

		if (ulReceivedValue == mainVALUE_SENT_FROM_TASK) //ako smo dobili podatak iz taksa inkrementira vrednost na zadnje dve cifre displeja
		{
			select_7seg_digit(4);
			set_7seg_digit(hexnum[a_num % 10]);
			select_7seg_digit(3);
			set_7seg_digit(hexnum[a_num / 10]);
			a_num++;
		}
		else if (ulReceivedValue == mainVALUE_SENT_FROM_TIMER)  //ako smo dobili podatak iz tajmera inkrementira vrednost na prve dve cifre displeja
		{
			select_7seg_digit(1);
			set_7seg_digit(hexnum[b_num % 10]);
			select_7seg_digit(0);
			set_7seg_digit(hexnum[b_num / 10]);
			b_num++;
		}
		else
		{
			printf("Unexpected message\r\n");
		}
	}
}

// MAIN - SYSTEM STARTUP POINT
void main_demo(void) 
{
	// INITIALIZATION OF THE PERIPHERALS
	init_7seg_comm();
	// ubacujemo crticu u srednju cifru
	select_7seg_digit(2);
	set_7seg_digit(0x40);//vrednost za crticu na cifri

	// TIMERS
	tH = xTimerCreate(NULL, pdMS_TO_TICKS(1100UL), pdTRUE, NULL, TimerCallback);
	if (tH == NULL)
		while (1);
	xTimerStart(tH, 0);


	// TASKS
	if (xTaskCreate(QueueReceive_tsk, "Rx", configMINIMAL_STACK_SIZE, NULL, 2, &tA) != pdPASS)
		while (1);// task za primanje podataka iz reda
	if (xTaskCreate(QueueSend_tsk, "TX", configMINIMAL_STACK_SIZE, NULL, 2, &tB) != pdPASS)
		while (1);// task za upis podataka u red

	//QUEUES
	myQueue = xQueueCreate(1, sizeof(uint32_t));// kreiramo Queue duzine dva uint32_t


	vTaskStartScheduler();
	while (1);
}