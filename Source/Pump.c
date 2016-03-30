//#define PUMP_TEST
/****************************************************************************
 Module
   Pump.c

 Revision
   1.0.0

 Description
   This is the service for interfacing with the rotary potentiometer that is 
   used like a pump by the user. It also implements the pump's state machine

 Notes

 History
 When           	Who     What/Why
 -------------- 	---     --------
 11/07/15 13:58 	JAM		first pass translating pseudo-code
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

#include "EventPrinter.h"

 #include "ADMulti.h"
 #include "Hose.h"
 #include "Pump.h"
 
 #include "termio.h"

#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)

 #define NUM_ANALOG_INPUTS 1 
 #define PUMP_INDEX 0 			// index of results array from analog input read
 #define PUMP_WAIT 1*ONE_SEC

#define LOWER_THRESHOLD 1400
 #define UPPER_THRESHOLD 1250

/*---------------------------- Module Functions ---------------------------*/
static void InitializeRotaryPin(uint8_t numInputs);
static uint32_t ReadPump(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static PumpState_t currentState;
static uint32_t lastPumpLevel;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitPump

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Takes care of hardware initialization of pump
 Notes

 Authors
	Jordan A. Miller,	11/07/15,	14:52
****************************************************************************/
bool InitPump(uint8_t Priority) {
	// Set priority
	MyPriority = Priority;

	// Call init function for analog pin associated with rotary pot
	InitializeRotaryPin(NUM_ANALOG_INPUTS);

	// Set currentState to INIT_PUMP
	currentState = INIT_PUMP;
	
	// Initialize the reading of the pump
	lastPumpLevel = ReadPump();

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
     PostPump

 Parameters
     ES_Event: the event to post to the pump service

 Returns
     bool: false if error in initialization, true otherwise

 Description
     Post events to the pump service
 Notes

 Authors
	Jordan A. Miller,	11/07/15,	14:54
****************************************************************************/
bool PostPump(ES_Event ThisEvent) {
	return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunPumpSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Pump processes events from the analog input of the rotary pot and inactive
   timeouts to determine the state of the pump and the ability for the hose
   to shoot.
 Notes
   
 Authors
	Jordan A. Miller,	11/07/15,	14:58
****************************************************************************/
ES_Event RunPump(ES_Event ThisEvent) {
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
	PumpState_t nextState = currentState;

	switch(currentState) {

		case INIT_PUMP:
			// if state is PUMP_IDLE
			if (ThisEvent.EventType == ES_INIT) {
				nextState = PUMP_IDLE;
			}
		break;

		case PUMP_IDLE:
			// if lower threshold is reached, change state to going up and
			// start the pump inactivity timer and tell the hose
			if (ThisEvent.EventType == LOWER_REACHED) {
				nextState = GOING_UP;
				ES_Timer_InitTimer(PUMP_TIMER, PUMP_WAIT);

				// Post the event to the hose
				ES_Event PumpEvent;
				PumpEvent.EventType = PUMP_STARTED;
				PostHose(PumpEvent);
				PostEventPrinter(PumpEvent);
				// ES_PostAll(PumpEvent);
			}

			// if upper threshold is reached, change state to going down and
			// start the pump inactivity timer and tell the hose
			if (ThisEvent.EventType == UPPER_REACHED) {
				nextState = GOING_DOWN;
				ES_Timer_InitTimer(PUMP_TIMER, PUMP_WAIT);

				// Post the event to the hose
				ES_Event PumpEvent;
				PumpEvent.EventType = PUMP_STARTED;
				PostHose(PumpEvent);
				PostEventPrinter(PumpEvent);
				// ES_PostAll(PumpEvent);
			}
		break;

	
		case GOING_UP:
			// if there is a timeout, set state to idle and tell hose
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == PUMP_TIMER) {
				nextState = PUMP_IDLE;

				// Post the event to the hose
				ES_Event PumpEvent;
				PumpEvent.EventType = PUMP_STOPPED;
				PostHose(PumpEvent);
				PostEventPrinter(PumpEvent);
				// ES_PostAll(PumpEvent);
			}

			// if the upper threshold is reached, start going down and 
			// restart the pump inactivity timer
			if (ThisEvent.EventType == UPPER_REACHED) {
				nextState = GOING_DOWN;
				ES_Timer_InitTimer(PUMP_TIMER,PUMP_WAIT);
			}

		break;

		case GOING_DOWN:
			// if there is a timeout, set state to idle and tell hose
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == PUMP_TIMER) {
				nextState = PUMP_IDLE;

				// Post the event to the hose
				ES_Event PumpEvent;
				PumpEvent.EventType = PUMP_STOPPED;
				PostHose(PumpEvent);
				PostEventPrinter(PumpEvent);
				// ES_PostAll(PumpEvent);
			}

			// if the lower threshold is reached, start going up and
			// restart the inactivity timer
			if (ThisEvent.EventType == LOWER_REACHED) {
				nextState = GOING_UP;
				ES_Timer_InitTimer(PUMP_TIMER,PUMP_WAIT);
			}

		break;
	}

	currentState = nextState;
	
	return ReturnEvent;
}

/****************************************************************************
 Function
    QueryPumpState

 Parameters
   none

 Returns
   PumpState_t, the current state of the pump

 Description
   Allows other services to query the state of the pump
 Notes
   
 Authors
	Jordan A. Miller,	11/07/15,	15:24
****************************************************************************/
PumpState_t QueryPump(void) {
	return currentState;
}

/****************************************************************************
 Function
    CheckPumpLevel

 Parameters
   none

 Returns
   bool, true if new event posted, false otherwise

 Description
   Checks for a change in the state of the pump by looking at an upper and
   lower threshold. Strokes of the pump must reach outside of this range in
   both directions of motion.
 Notes
   
 Authors
	Jordan A. Miller,	11/07/15,	15:34
****************************************************************************/
bool CheckPumpLevel(void) {
	bool returnVal = false;

	// sample analog pin for rotary pot and set to currentPumpLevel
	uint32_t currentPumpLevel = ReadPump();

	// if the pump reaches its upper threshold, tell pump SM
	if (currentPumpLevel > UPPER_THRESHOLD && lastPumpLevel <= UPPER_THRESHOLD && currentState != GOING_DOWN) {
		ES_Event ThisEvent;
		ThisEvent.EventType = UPPER_REACHED;
//		printf("currentPumpLevel = %u\r\n",currentPumpLevel);
//		printf("lastPumpLevel = %u\r\n",lastPumpLevel);
		// ES_PostAll(ThisEvent);
		PostPump(ThisEvent);
		PostEventPrinter(ThisEvent);
		returnVal = true;
	}

	// if the pump reaches its lower threshold, tell pump SM
	else if (currentPumpLevel < LOWER_THRESHOLD && lastPumpLevel >= LOWER_THRESHOLD && currentState != GOING_UP) {
		ES_Event ThisEvent;
		ThisEvent.EventType = LOWER_REACHED;
//		printf("currentPumpLevel = %u\r\n",currentPumpLevel);
//		printf("lastPumpLevel = %u\r\n",lastPumpLevel);
		// ES_PostAll(ThisEvent);
		PostPump(ThisEvent);
		PostEventPrinter(ThisEvent);
		returnVal = true;
	}
	
	lastPumpLevel = currentPumpLevel;
	return returnVal;

}


/***************************************************************************
 private functions
 ***************************************************************************/

 /****************************************************************************
 Function
     InitializeRotaryPin

 Parameters
     uint8_t : numInputs, the number of analog input to be initialized

 Returns
     none

 Description
     Initializes the PE0 pin to accept analog input from the rotary pot
 Notes

 Author
     Jordan Miller, 11/07/15, 14:25
****************************************************************************/
static void InitializeRotaryPin(uint8_t numInputs) {
	// use given function for initializing analog input ports
	ADC_MultiInit(numInputs);

}

 /****************************************************************************
 Function
     ReadPump

 Parameters
     none

 Returns
     uint32_t representing analog voltage

 Description
     Reads the voltage at the rotary pin and returns a 12-bit representation
     of the current input voltage.
 Notes

 Author
     Jordan Miller, 11/07/15, 14:36
****************************************************************************/
static uint32_t ReadPump() {
	static uint32_t results[NUM_ANALOG_INPUTS];

	ADC_MultiRead(results);

	return results[PUMP_INDEX];
}

	
#ifdef PUMP_TEST
#include "EnablePA25_PB23_PD7_PF0.h"

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
	
	uint32_t testValue;
	InitializeRotaryPin(1);
	
	while (true) {
		testValue = ReadPump();
		printf("Analog Input Value %u\r\n", testValue);
		getchar();
	}
	
	return 0;
}

#endif

	
