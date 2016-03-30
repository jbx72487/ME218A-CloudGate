//#define STIR_TEST
/****************************************************************************
 Module
   Stir.c

 Revision
   1.0.0

 Description
   This is the service for interfacing with the digital encoder (coin
   sensor) that is used like a stirrer by the user. It also implements
   the stirring state machine.

 Notes

 History
 When           	Who     What/Why
 -------------- 	---     --------
 11/08/15	19:08		JAM			debugging in uVision
 11/07/15 18:23 	JAM			first pass translating pseudo-code
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

#include "Stir.h"

 #include "termio.h"

#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define TENTH_SEC (ONE_SEC/10)
#define STIR_WAIT 1*ONE_SEC

#define ENCODER_HI BIT1HI 
#define ALL_BITS (0xff<<2)

/*---------------------------- Module Functions ---------------------------*/
static void InitializeEncoderPin(void);
static uint8_t ReadStir(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static StirState_t currentState;
static uint8_t lastStirState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitStir

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Takes care of hardware initialization of stir
 Notes

 Authors
	Jordan A. Miller,	11/07/15,	18:43
****************************************************************************/
bool InitStir(uint8_t Priority) {

	MyPriority = Priority;

	//initialize encoder pin to accept digital input
	InitializeEncoderPin();

	// Set currentState to STIR_INIT
	currentState = STIR_INIT;

	// sample encoder pin and set equal to lastStirState
	lastStirState = ReadStir();

	// Post the initial transition event
	ES_Event ThisEvent;
	ThisEvent.EventType = ES_INIT;
  	if (ES_PostToService( MyPriority, ThisEvent) == true) {  
    	return true;
  	}
	else {
    	return false;
  	}	
}

/****************************************************************************
 Function
     PostStir

 Parameters
     ES_Event: the event to post to the stir service

 Returns
     bool: false if error in posting, true otherwise

 Description
     Post events to the stir service
 Notes

 Authors
	Jordan A. Miller,	11/07/15,	18:46
****************************************************************************/
bool PostStir(ES_Event ThisEvent) {
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunStir

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Stir processes events from the input of the digital encoder and inactive
   timeouts to determine the state of the stir and the to mix chemicals.
 Notes
   
 Authors
	Jordan A. Miller,	11/07/15,	18:51
****************************************************************************/
ES_Event RunStir(ES_Event ThisEvent) {
	ES_Event ReturnEvent;
	ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

	StirState_t nextState = currentState;

	switch(currentState) {
		case STIR_INIT:
			if (ThisEvent.EventType == ES_INIT) {
				nextState = STIR_IDLE;
			}
		break;

		
		case STIR_IDLE:
			if (ThisEvent.EventType == STIR_ACTIVE) {
				nextState = STIRRING;
				ES_Timer_InitTimer(STIR_TIMER,STIR_WAIT);
				//printf("STIR ACTIVE\r\n\n");
			}
		break;

		case STIRRING:
			// if event is ES_TIMEOUT
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == STIR_TIMER) {
				nextState = STIR_IDLE;
				//printf("STIR IDLE\r\n\n");
			}

			// if event is STIR_ACTIVE
			if (ThisEvent.EventType == STIR_ACTIVE) {
				nextState = STIRRING;
				ES_Timer_InitTimer(STIR_TIMER,STIR_WAIT);
			}
		break;
	}

	currentState = nextState;

	return ReturnEvent;
}

/****************************************************************************
 Function
    QueryStir

 Parameters
   none

 Returns
   StirState_t : the current state of the stirring SM

 Description
   Allows other services to query the state of the stirring state machine
 Notes
   
 Authors
	Jordan A. Miller,	11/07/15,	18:58
****************************************************************************/
StirState_t QueryStir(void) {
	return currentState;
}

/****************************************************************************
 Function
   CheckStirState

 Parameters
   none

 Returns
   bool : true if event posted, false otherwise

 Description
   Actively checks for a change in the state of the encoder to confirm that 
   the stirring is still active.
 Notes
   
 Authors
	Jordan A. Miller,	11/07/15,	18:59
****************************************************************************/
bool CheckStirState(void) {
	bool returnVal = false;

	// Sample encoder state and set to currentStirState
	uint8_t currentStirState = ReadStir();
	//printf("Event : Stir State: 0x%x\r\n\n",currentStirState);
	
	if (currentStirState != lastStirState) {
		
		ES_Event ThisEvent;
		ThisEvent.EventType = STIR_ACTIVE;
		//PostStir(ThisEvent);
		ES_PostAll(ThisEvent);

		returnVal = true;
	}
	
	lastStirState = currentStirState;

	return returnVal;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void InitializeEncoderPin(void) {
	// if encoder port (PBO) is not turned on turn it on and wait
  	if((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1) {
    	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
    	while((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
 	}
	
	// Set it to be a digitial I/O pin
	HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= ENCODER_HI;

  	// Set encoder as an input pin
  	HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) &= ~ENCODER_HI;
}

static uint8_t ReadStir(void) {
  return (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) & ENCODER_HI);
}

#ifdef STIR_TEST
int main(void) {

	
	
	return 0;
}	
#endif

