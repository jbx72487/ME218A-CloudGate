/****************************
* EventGenerator responds to keystrokes on the computer screen to send events to other functions
****************************/

/************* Included Headers ****************/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "EventGenerator.h"
#include <stdio.h>

// TODO #include headers of any relevant events to post to
#include "Gameplay.h"
#include "Pump.h"
#include "LEDControl.h"
/************* Module Defines ****************/

/************* Private Functions ****************/
void ReportAllStates(void);

/************* Module Variables ****************/
static uint8_t MyPriority;



bool InitEventGenerator(uint8_t Priority) {
	MyPriority = Priority;
	return true;
}

bool PostEventGenerator(ES_Event ThisEvent) {
	return ES_PostToService(MyPriority,ThisEvent);
}

ES_Event RunEventGenerator(ES_Event ThisEvent) {
	ES_Event ReturnVal;
	ReturnVal.EventType = ES_NO_EVENT;
	
	if (ThisEvent.EventType == ES_NEW_KEY) {
		ES_Event Event2Post;
		switch (ThisEvent.EventParam) {
			case 'a':
				Event2Post.EventType = START_BUTTON_UP;
				ES_PostAll(Event2Post);
				break;
/*			case 'b':
				Event2Post.EventType = ES_TIMEOUT;
				Event2Post.EventParam = GAME_STATE_TIMER;
				ES_PostAll(Event2Post);
				break;
*/
			case 'b':
				Event2Post.EventType = CHEM_BUTTON_DOWN;
				Event2Post.EventParam = 0;
				ES_PostAll(Event2Post);
				Event2Post.EventType = CHEM_BUTTON_UP;
				ES_PostAll(Event2Post);
				break;
			case 'c':
				Event2Post.EventType = CHEM_BUTTON_DOWN;
				Event2Post.EventParam = 1;
				ES_PostAll(Event2Post);
				Event2Post.EventType = CHEM_BUTTON_UP;
				ES_PostAll(Event2Post);
				break;
			case 'd':
				Event2Post.EventType = FIRE_READY;
				ES_PostAll(Event2Post);
				break;
			case 'e':
				Event2Post.EventType = FIRE_ON;
				ES_PostAll(Event2Post);
				break;
			case 'f':
				Event2Post.EventType = FIRE_OFF;
				ES_PostAll(Event2Post);
				break;
			case 'g':
				Event2Post.EventType = USED_ONE_AMMO;
				ES_PostAll(Event2Post);
				break;
			case 'h':
				Event2Post.EventType = CITY_SAVED;
				ES_PostAll(Event2Post);
				break;
			case 'i':
				Event2Post.EventType = ALL_CITIES_SAVED;
				ES_PostAll(Event2Post);
				break;
			case 'j':
				Event2Post.EventType = STIR_ACTIVE;
				ES_PostAll(Event2Post);
				break;
			case 'k':
				Event2Post.EventType = LOWER_REACHED;
				ES_PostAll(Event2Post);
				break;
			case 'l':
				Event2Post.EventType = UPPER_REACHED;
				ES_PostAll(Event2Post);
				break;
			case 'm':
				Event2Post.EventType = TRIGGER_DOWN;
				ES_PostAll(Event2Post);
				break;
			case 'n':
				Event2Post.EventType = TRIGGER_UP;
				ES_PostAll(Event2Post);
				break;
			case 'o':
				Event2Post.EventType = TURN_IR_ON;
				ES_PostAll(Event2Post);
				break;
			case '0': // turn on all red LEDs
				setLED( LED_ALL_CITIES_RED , OFF);
				setLED( LED_ALL_CITIES_BLUE, ON);
				printf("0 Input. All City LEDs set to red. 0x%x\r\n",BIT31HI);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
				// Turn off that city’s red LEDs ring
				setLED(LED_SACRAMENTO_RED << (ThisEvent.EventParam - 1), OFF);
				// Turn on that city’s blue LEDs ring
				setLED(LED_SACRAMENTO_BLUE<< (ThisEvent.EventParam - 1), ON);
				break;
			case '?':
				ReportAllStates();
			
				break;
		}
	}
	
	return ReturnVal;
}

void ReportAllStates() {
	printf("Gameplay: ");
	switch (QueryGameplay()) {
		case INIT:
			printf("INIT\r\n");
			break;
		case ARMED:
			printf("ARMED\r\n");
			break;
		case MIXING:
			printf("MIXING\r\n");
			break;
		case SHOOTING:
			printf("SHOOTING\r\n");
			break;
		case FAIL:
			printf("FAIL\r\n");
			break;
		case WIN:
			printf("WIN\r\n");
			break;
	}
	
	switch(QueryPump()) {
		case INIT_PUMP:
			printf("Pump State: INIT_PUMP\r\n");
			break;
		case PUMP_IDLE:
			printf("Pump State: PUMP_IDLE\r\n");
			break;
		case GOING_UP:
			printf("Pump State: GOING_UP\r\n");
			break;
		case GOING_DOWN:
			printf("Pump State: GOING_DOWN\r\n");
			break;
	}
	
	printf("\n\r");
	/*printf("MorseElementsSM: ");
	switch (QueryMorseElementsSM()) {
		case InitMorseElements:
			printf("InitMorseElements");
			break;
		case CalWaitForRise:
			printf("CalWaitForRise");
			break;
		case CalWaitForFall:
			printf("CalWaitForFall");
			break;
		case EOC_WaitRise:
			printf("EOC_WaitRise");
			break;
		case EOC_WaitFall:
			printf("EOC_WaitFall");
			break;
		case DecodeWaitRise:
			printf("DecodeWaitRise");
			break;
		case DecodeWaitFall:
			printf("DecodeWaitFall");
			break;
	}
	printf("\n\r");
	*/
}

