/********************* Included headers *******************/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "EventPrinter.h"
#include <stdio.h>

/******************** Module Defines ***********************/


/******************** Private Functions ***********************/

/******************** Module Variables ***********************/
static uint8_t MyPriority;

/******************** Module Code ***********************/
bool InitEventPrinter(uint8_t Priority) {
	MyPriority = Priority;
	return true;
}

bool PostEventPrinter(ES_Event ThisEvent) {
	if (ES_PostToService(MyPriority, ThisEvent) == true)
		return true;
	else
		return false;
}

// prints messages to the screen depending on which event is happening

ES_Event RunEventPrinter(ES_Event ThisEvent) {
	ES_Event ReturnVal;
	ReturnVal.EventType = ES_NO_EVENT;
	
	switch (ThisEvent.EventType) {
		case ES_TIMEOUT:
			switch (ThisEvent.EventParam) {
				case GAME_TIMER:
					printf("Timer expired: GAME_TIMER\r\n");
					break;
				case INACTION_TIMER:
					printf("Timer expired: INACTION_TIMER\r\n");
					break;
				case GAME_STATE_TIMER:
					printf("Timer expired: GAME_STATE_TIMER\r\n");
					break;
				case PUMP_TIMER:
					printf("Timer expired: PUMP_TIMER\r\n");\
					break;
				case TIME_OF_DAY_TIMER:
					printf("Timer expired: TIME_OF_DAY_TIMER\r\n");
					break;
/*				case LED_FLASH_TIMER:
					printf("Timer expired: LED_FLASH_TIMER\r\n");
					break; */
			}
			break;
		case  START_BUTTON_DOWN:
			printf("\nEvent: START_BUTTON_DOWN \n\r");
			break;
		case  START_BUTTON_UP:
			printf("\nEvent: START_BUTTON_UP \n\r");
			break;
		case CHEM_BUTTON_DOWN:
			printf("\nEvent: CHEM_BUTTON_DOWN: chemical %u \n\r",ThisEvent.EventParam);
			break;
		case CHEM_BUTTON_UP:
			printf("\nEvent: CHEM_BUTTON_UP: chemical %u \n\r",ThisEvent.EventParam);
			break;
		case TRIGGER_DOWN:
			printf("\nEvent: TRIGGER_DOWN \n\r");
			break;
		case TRIGGER_UP:
			printf("\nEvent: TRIGGER_UP \n\r");
			break;
		case PUMP_STOPPED:
			printf("\nEvent: PUMP_STOPPED \n\r");
			break;
        case PUMP_STARTED:
			printf("\nEvent: PUMP_STARTED \n\r");
			break;
        case FIRE_ON:
			printf("\nEvent: FIRE_ON \n\r");
			break;
        case FIRE_OFF:
			printf("\nEvent: FIRE_OFF \n\r");
			break;
        case FIRE_READY:
			printf("\nEvent: FIRE_READY \n\r");
			break;
        case USED_ONE_AMMO:
			printf("\nEvent: USED_ONE_AMMO \n\r");
			break;
        case CITY_SAVED:
			printf("\nEvent: CITY_SAVED \n\r");
			break; 
        case STIR_ACTIVE:
			printf("\nEvent: STIR_ACTIVE \n\r");
			break; 
        case LOWER_REACHED:
			printf("\nEvent: LOWER_REACHED \n\r");
			break;
        case UPPER_REACHED:
			printf("\nEvent: UPPER_REACHED \n\r");
			break;
				
		case ALL_CITIES_SAVED:
			printf("\nEvent: ALL_CITIES_SAVED \n\r");
			break;
		case TURN_IR_ON:
			printf("\nEvent: TURN_IR_ON \n\r");
			break;
		case TURN_IR_OFF:
			printf("\nEvent: TURN_IR_OFF \n\r");
			break;
	}

	return ReturnVal;
}
