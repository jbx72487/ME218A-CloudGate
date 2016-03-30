//#define IR_PULSE_TEST
/****************************************************************************
 Module
   IR_Pulse.c

 Revision
   1.0.0

 Description
   This is a very simple service to pulse the IR when it is turned on
 Notes

 History
 When           	Who     What/Why
 -------------- 	---     --------
 11/08/15 02:00 	JAM			first pass
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
 
 #include "IR_Pulse.h"
 #include "PWM_Control.h"

#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

/*----------------------------- Module Defines ----------------------------*/
 #define PERIOD 10000

/*---------------------------- Module Functions ---------------------------*/


/*---------------------------- Module Variables ---------------------------*/
static bool currentIRState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     PulseInit

 Parameters
     none

 Returns
     none

 Description
     Initializes the pulse output pin 
 Notes

 Authors
	Jordan A. Miller,	11/08/15,	14:52
****************************************************************************/
void PulseInit(void) {

	PWMInit();
	currentIRState = false;
	PWM8_TIVA_SetPeriod(PERIOD,2);	// Group 2 includes PE4 and PE5
	
}

/****************************************************************************
 Function
     TurnIROn

 Parameters
     none

 Returns
     none

 Description
     Pulses the IR output to simulate being on 
 Notes

 Authors
Jordan A. Miller,	11/10/15,	02:01
****************************************************************************/
void TurnIROn(void) {
	PWM8_TIVA_SetDuty(50,PULSE_CHANNEL);
	currentIRState = true;
}

/****************************************************************************
 Function
     TurnIROff

 Parameters
     none

 Returns
     none

 Description
     Turns the IR pulse completely off 
 Notes

 Authors
Jordan A. Miller,	11/10/15,	02:0f
****************************************************************************/
void TurnIROff(void) {
	PWM8_TIVA_SetDuty(0,PULSE_CHANNEL);
	currentIRState = false;
}


/****************************************************************************
 Function
     GetIRState

 Parameters
     none

 Returns
     boolean indicating state of the IR emitter

 Description
     Returns the current state of the IR emitter
 Notes

 Authors
Joy Xi,	11/17/15,	02:0f
****************************************************************************/
bool GetIRState(void) {
	return currentIRState;
}



/***************************************************************************
 private functions
 ***************************************************************************/	
#ifdef IR_PULSE_TEST
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
	

}

#endif

	
