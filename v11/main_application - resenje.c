// STANDARD INCLUDES
#include <stdio.h>l.g 
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// KERNEL INCLUDES
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "extint.h"

// HARDWARE SIMULATOR UTILITY FUNCTIONS  
#include "HW_access.h"


// SERIAL SIMULATOR CHANNEL TO USE 
#define COM_CH_0 (0)


// TASK PRIORITIES 
#define TASK_SERIAl_REC_PRI			( tskIDLE_PRIORITY + 2 )
#define	SERVICE_TASK_PRI			( tskIDLE_PRIORITY + 1 )


// TASKS: FORWARD DECLARATIONS 
void Disp_Task(void* pvParameters);
void SerialReceive_Task(void* pvParameters);


// RECEPTION DATA BUFFER - COM 0
#define R_BUF_SIZE (3)
uint8_t r_buffer[R_BUF_SIZE];
unsigned volatile r_point;


// 7-SEG NUMBER DATABASE - ALL HEX DIGITS [ 0 1 2 3 4 5 6 7 8 9 A B C D E F ]
static const char hexnum[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };


// GLOBAL OS-HANDLES 
SemaphoreHandle_t RXC_BinarySemaphore;
SemaphoreHandle_t Disp_BinarySemaphore;
QueueHandle_t Data_Queue;


// STRUCTURES


// INTERRUPTS //



static uint32_t prvProcessRXCInterrupt(void) {	// RXC - RECEPTION COMPLETE - INTERRUPT HANDLER 
	BaseType_t xHigherPTW = pdFALSE;

	if (get_RXC_status(COM_CH_0) != 0)
		xSemaphoreGiveFromISR(RXC_BinarySemaphore, &xHigherPTW);

	portYIELD_FROM_ISR(xHigherPTW);
}


// MAIN - SYSTEM STARTUP POINT 
void main_demo(void) {
	// INITIALIZATION OF THE PERIPHERALS
	init_7seg_comm();
	//init_serial_uplink(COM_CH_0);		// inicijalizacija serijske TX na kanalu 0
	init_serial_downlink(COM_CH_0);	// inicijalizacija serijske RX na kanalu 0


	// INTERRUPT HANDLERS
	vPortSetInterruptHandler(portINTERRUPT_SRL_RXC, prvProcessRXCInterrupt);	// SERIAL RECEPTION INTERRUPT HANDLER 

	// BINARY SEMAPHORES
	RXC_BinarySemaphore = xSemaphoreCreateBinary();		// CREATE RXC SEMAPHORE - SERIAL RECEIVE COMM
	Disp_BinarySemaphore = xSemaphoreCreateBinary();
	// QUEUES
	Data_Queue = xQueueCreate(1, sizeof(uint8_t));

	// TASKS 
	xTaskCreate(SerialReceive_Task, "SRx", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAl_REC_PRI, NULL);	// SERIAL RECEIVER TASK 
	r_point = 0;
	xTaskCreate(Disp_Task, "disp", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);				// CREATE LED BAR TASK  


	// START SCHEDULER
	vTaskStartScheduler();
	while (1);
}

// TASKS: IMPLEMENTATIONS
void Disp_Task(void* pvParameters) {
	uint8_t tmp[R_BUF_SIZE];
	uint8_t podatak; 
	while (1) {

		//xSemaphoreTake(Disp_BinarySemaphore, portMAX_DELAY);
		xQueueReceive(Data_Queue, &podatak, portMAX_DELAY);
/*
		printf("p1 : %u\n", (unsigned)tmp[0]);
		printf("p2 : %u\n", (unsigned)tmp[1]);
		printf("p3 : %u\n", (unsigned)tmp[2]);
		podatak = tmp[0] * 100 + tmp[1] * 10 + tmp[2];
		*/
		printf("Podatak : %u\n", (unsigned)podatak);

		podatak = (-100 / (float)68) * ((int)podatak - 68);
		podatak = (uint8_t)podatak;
		printf("Konvertovano : %u\n", (unsigned)podatak);

		select_7seg_digit(2);
		set_7seg_digit(hexnum[podatak % 10]);
		select_7seg_digit(1);
		set_7seg_digit(hexnum[(podatak / 10) % 10]);
		select_7seg_digit(0);
		set_7seg_digit(hexnum[podatak / 100]);
	}
}


void SerialReceive_Task(void* pvParameters) {
	uint8_t cc = 0;
	uint8_t podatak;
	while (1) {

		xSemaphoreTake(RXC_BinarySemaphore, portMAX_DELAY);// ceka na serijski prijemni interapt
		get_serial_character(COM_CH_0, &cc);//ucitava primljeni karakter u promenjivu cc
		printf("KANAL 0: primio karakter: %u\n", (unsigned)cc);// prikazuje primljeni karakter u cmd prompt

		if (cc == 0x0d) {	// za svaki KRAJ poruke, prikazati primljenje bajtove direktno na displeju 3-4
			podatak = r_buffer[0] * 100 + r_buffer[1] * 10 + r_buffer[2];

			xQueueSend(Data_Queue, &podatak, portMAX_DELAY);
			//xSemaphoreGive(Disp_BinarySemaphore);
			r_point = 0;
		}
		else if (r_point < R_BUF_SIZE) { // pamti karaktere izmedju EF i FF
			r_buffer[r_point++] = cc - '0';
		}
	}
}