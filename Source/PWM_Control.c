//#define PWM_TEST
/****************************************************************************
 Module
   PWM_Control.c

 Revision
   1.0.0

 Description
   This is a very simple service to simplify PWM initialization. It is called by 
	 servos and the pulsed IR pin

 Notes

 History
 When           	Who     What/Why
 -------------- 	---     --------
 11/10/15 01:51 	JAM			first pass
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
 
 #include "termio.h"
 
 #include "PWM8Tiva.h"

#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/
static bool PWM_Initialized = false;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     PWMInit

 Parameters
     none

 Returns
     none

 Description
     Initializes the 8 PWM pins
 Notes

 Authors
	Jordan A. Miller,	11/08/15,	14:52
****************************************************************************/
void PWMInit(void) {
	// Call init function for analog pin associated with rotary pot
	if (PWM_Initialized == false) {
		PWM8_TIVA_Init();
		PWM_Initialized = true;
	}
}

/***************************************************************************
 private functions
 ***************************************************************************/
	
#ifdef PWM_TEST

int main(void) {
	PortFunctionInit();
	TERMIO_Init();
	clrScrn();
	
	// When doing testing, it is useful to announce just which program
	// is running.
	puts("\rStarting Test Harness for \r");
	printf("the 2nd Generation Events & Services Framework V2.2\r\n");
	printf("%s %s\n",__TIME__, __DATE__);
	printf("\n\r\n");
	printf("Press any key to post key-stroke events to Service 0\n\r");
	printf("Press 'd' to test event deferral \n\r");
	printf("Press 'r' to test event recall \n\n\r");
	
	PWMInit();

	
	return 0;
}

#endif

	
