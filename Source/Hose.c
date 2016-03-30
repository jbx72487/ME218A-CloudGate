//#define HOSE_TEST
/********************* Included headers *******************/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_ShortTimer.h"
#include "Hose.h"
#include "Pump.h"
#include "IR_Pulse.h"
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "Gameplay.h"
#include "Eventprinter.h"
#include "EnablePA25_PB23_PD7_PF0.h"

/******************** Module Defines ***********************/

#define ONE_SEC 976
#define AMMO_TIME 1*ONE_SEC

#define IR_PULSE_TIMER TIMER_A
#define IR_PULSE_TIME 1000

#define ALL_BITS (0xff<<2)

/******************** Private Functions ***********************/


/******************** Module Variables ***********************/
static uint8_t MyPriority;
static HoseState_t currentState;

/******************** Module Code ***********************/
bool InitHose(uint8_t Priority) {
	MyPriority = Priority;
	// initialize Tiva hardware: Port B, pin 3
	
	PulseInit();
	
	currentState = HOSE_INIT;
	ES_Event ThisEvent;
	ThisEvent.EventType = ES_INIT;

	// Initialize short timer to pulse IR
  //ES_ShortTimerInit(MyPriority, SHORT_TIMER_UNUSED );
	
	// post ES_INIT to itself
	if (ES_PostToService(MyPriority, ThisEvent) == true)
	{
		return true;
	} else {
		return false;
	}
}

bool PostHose(ES_Event ThisEvent) {
	if (ES_PostToService(MyPriority, ThisEvent) == true)
		return true;
	else
		return false;
}

ES_Event RunHose(ES_Event ThisEvent) {
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;
	HoseState_t nextState = currentState;
			
	switch (currentState) {
		case HOSE_INIT:
				if (ThisEvent.EventType == ES_INIT) {
					// next state is idle
					nextState = HOSE_IDLE;
					
					// set the ammo timer
					ES_Timer_SetTimer(AMMO_TIMER, AMMO_TIME);				
				}
		break;
			
		case HOSE_IDLE:
				// if pump is active without pulling trigger, tell Gameplay we’re ready to shoot
				if (ThisEvent.EventType == PUMP_STARTED) {
					ES_Event Event2Post;
					Event2Post.EventType = FIRE_READY;
					PostGameplay(Event2Post);
					PostEventPrinter(Event2Post);
					// ES_PostAll(Event2Post);

					// TODO change to only post to Gameplay after debugging
				}
				
				// if trigger is pulled and pump is active
				if (ThisEvent.EventType == TRIGGER_DOWN && QueryPump() != PUMP_IDLE) {
					// go to HOSE_SHOOTING state
					nextState = HOSE_SHOOTING;
					
					//tell Gameplay we’re firing
					ES_Event Event2Post;
					Event2Post.EventType = FIRE_ON;
					PostGameplay(Event2Post);
					PostEventPrinter(Event2Post);
					// ES_PostAll(Event2Post);

					// TODO change to only post to Gameplay after debugging
					
					// start the ammo timer
					ES_Timer_StartTimer(AMMO_TIMER);
					
					// Turn on IR and start pulse timer
					TurnIROn();
					//ES_ShortTimerStart(IR_PULSE_TIMER,IR_PULSE_TIME);					
				}
				
		break;
			
		case HOSE_SHOOTING:
			
				// if ammo timer expires
				if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == AMMO_TIMER) {
					// Continue shooting
					nextState = HOSE_SHOOTING;
					
					// tell Gameplay we’ve used up one ammo
					ES_Event Event2Post;
					Event2Post.EventType = USED_ONE_AMMO;
					PostGameplay(Event2Post);
					PostEventPrinter(Event2Post);
					// ES_PostAll(Event2Post);

					// TODO change to only post to Gameplay after debugging		
					
					// Start the next ammo timer
					ES_Timer_InitTimer(AMMO_TIMER, AMMO_TIME);
				}
				
				// if trigger is released
				if (ThisEvent.EventType == TRIGGER_UP) {
					// Go back to idle state
					nextState = HOSE_IDLE;
					
					// Tell Gameplay we’re ready to shoot
					ES_Event Event2Post;
					Event2Post.EventType = FIRE_READY;
					PostGameplay(Event2Post);
					PostEventPrinter(Event2Post);
					// ES_PostAll(Event2Post);
					
					// TODO change to only post to Gameplay after debugging
					
					// Stop ammo timer
					ES_Timer_StopTimer(AMMO_TIMER);
					
					// Turn IR off and stop the pulse timer
					TurnIROff();
					//ES_Timer_StopTimer(IR_PULSE_TIMER);
				}
				
				// if user stops pumping
				if (ThisEvent.EventType == PUMP_STOPPED) {
					// Go back to idle state
					nextState = HOSE_IDLE;
					
					// tell Gameplay we’re not ready to shoot, stop the ammo timer, and go back to HOSE_IDLE
					ES_Event Event2Post;
					Event2Post.EventType = FIRE_OFF;
					PostGameplay(Event2Post);
					PostEventPrinter(Event2Post);
					// ES_PostAll(Event2Post);

					// TODO change to only post to Gameplay after debugging 
					
					// Turn the IR off and stop pulse timer
					TurnIROff();
					//ES_Timer_StopTimer(IR_PULSE_TIMER);
					
					// Stop ammo timer
					ES_Timer_StopTimer(AMMO_TIMER);
				}
				
		break;
			
	}
	
	currentState = nextState;
	return returnVal;
}


#ifdef HOSE_TEST
#include "termio.h"
#include "EnablePA25_PB23_PD7_PF0.h"

#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

int main(void) {
		// Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
			| SYSCTL_XTAL_16MHZ);
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
	printf("Press 'r' to test event recall \n\r");
	InitializeIRPin();
	
	TurnIROn();
	while (true) {
		printf("Toggle\r\n");
		getchar();
		TurnIROff();
		getchar();
		TurnIROn();
		//ToggleIR();
	}
	
	return 0;
}
#endif
