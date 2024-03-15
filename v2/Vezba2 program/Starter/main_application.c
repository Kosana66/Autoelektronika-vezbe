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

/* Local function declaration */
static void DisplayDigit_0();

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


// MAIN - SYSTEM STARTUP POINT 
void main_demo(void) {
	// INITIALIZATION OF THE PERIPHERALS
	init_7seg_comm();

	// TASKS 

	// TIMERS

	// SEMAPHORES

	// START SCHEDULER
	vTaskStartScheduler();
}