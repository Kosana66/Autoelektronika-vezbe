
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


#define mainVALUE_SENT_FROM_TASK			( 100UL )
#define mainVALUE_SENT_FROM_TIMER			( 200UL )


// Local function declaration


// Local function implementation



// MAIN - SYSTEM STARTUP POINT
void main_demo(void)
{
	// INITIALIZATION OF THE PERIPHERALS
	init_7seg_comm();

	// ubacujemo crticu u srednju cifru
	select_7seg_digit(2);
	set_7seg_digit(0x40);//vrednost za crticu na cifri


	// TIMERS
	


	// TASKS
	


	//QUEUES
	


	// Pokretanje rasporedjivaca
	vTaskStartScheduler();
	while (1);
}