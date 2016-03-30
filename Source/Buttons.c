

/********************* Included headers *******************/
#include "ES_Configure.h"
#include "ES_Framework.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"

#include "BITDEFS.H"
#include "ES_Port.h"
#include "EnablePA25_PB23_PD7_PF0.h"

#include "Buttons.h"
#include "Hose.h"
#include "EventPrinter.h"
#include "Gameplay.h"
#include <stdio.h>


/******************** Module Defines ***********************/
#define ALL_BITS (0xff<<2)

#define BUTTON_TIME 20 // this is a number which should be longer than debouncing period but shorter than length of a button press
#define TRIGGER_MASK GPIO_PIN_4
#define START_MASK GPIO_PIN_5
#define CHEMA_MASK GPIO_PIN_7
#define CHEMB_MASK GPIO_PIN_6

/******************** Private Functions ***********************/


/******************** Module Variables ***********************/
static uint8_t MyPriority;
static uint8_t lastButtonStates;

/******************** Module Code ***********************/
bool InitButtons(uint8_t Priority) {

	MyPriority = Priority;
	// initialize Tiva hardware: Port C, pins 4,5,6,7

	//enable the clock to Port C by setting bit 3 in register
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_PRGPIO_R2;

	// wait for clock to be ready
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R2) != SYSCTL_PRGPIO_R2)
	;

	// Connect Pin 0,1,2,3,6,7 to digital I/O port by writing to Digital Enable register
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= (TRIGGER_MASK | START_MASK | CHEMA_MASK | CHEMB_MASK);

	// Set data direction to input for these pins by writing them LOW to direction register
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= ~(TRIGGER_MASK | START_MASK | CHEMA_MASK | CHEMB_MASK);


	HWREG(GPIO_PORTC_BASE+GPIO_O_CR) |= (TRIGGER_MASK | START_MASK | CHEMA_MASK | CHEMB_MASK);
	
	// active low, so configure pins with pull-up resistors
	HWREG(GPIO_PORTC_BASE+GPIO_O_PUR) |= (TRIGGER_MASK | START_MASK | CHEMA_MASK | CHEMB_MASK);
	
/*
	// wait for pull-ups to be ready
	while ((HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) & (TRIGGER_MASK | START_MASK | CHEMA_MASK | CHEMB_MASK)) != (TRIGGER_MASK | START_MASK | CHEMA_MASK | CHEMB_MASK))
	;
	*/
	// read in initial values to be lastButtonStates
	// lastButtonStates = HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) & (TRIGGER_MASK | START_MASK | CHEMA_MASK | CHEMB_MASK);
	lastButtonStates = 127;
	// printf("lastButtonStates: %#08x\r\n", lastButtonStates);
	
	ES_Timer_InitTimer(BUTTON_TIMER,BUTTON_TIME);
	return true;
}

bool PostButtons(ES_Event ThisEvent) {
	if (ES_PostToService(MyPriority, ThisEvent) == true)
		return true;
	else
		return false;
}


ES_Event RunButtons(ES_Event ThisEvent) {
	//printf("Running runButton!\r\n");
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;
	// if BUTTON_TIMER runs out then
	if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == BUTTON_TIMER) {
		checkButtonEvents();
		// Start BUTTON_TIMER
		ES_Timer_InitTimer(BUTTON_TIMER,BUTTON_TIME);
	}
	return returnVal;
}


bool checkButtonEvents(void) {

	bool returnVal = false;
	//printf("currentButtonStates: %#08x\r\n", lastButtonStates);

	// Get all button states,
                                                                                                                    
	uint8_t currentButtonStates = HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) & (TRIGGER_MASK | START_MASK | CHEMA_MASK | CHEMB_MASK);
	if (lastButtonStates == 127) lastButtonStates = currentButtonStates;
	// If any changed since last time and if relevant, 
	// Post START_BUTTON_UP, CHEM_BUTTON_UP, CHEM_BUTTON_DOWN, TRIGGER_UP, TRIGGER_DOWN

	// if these set of button values is diff from the last
	if (lastButtonStates != currentButtonStates) {
		ES_Event Event2Post;
		returnVal = true;
		printf("Button states changed/r/n");
		
		// if trigger is diff from last time
		if ((currentButtonStates & TRIGGER_MASK) != (lastButtonStates & TRIGGER_MASK)) {
			// if  it's low, then send a TRIGGER_DOWN event, otherwise send TRIGGER_UP
			if ((currentButtonStates & TRIGGER_MASK) == 0) {
				Event2Post.EventType = TRIGGER_DOWN;
				// ES_PostAll(Event2Post);
				PostHose(Event2Post);
				PostEventPrinter(Event2Post);
			} else {
				Event2Post.EventType = TRIGGER_UP;
				// ES_PostAll(Event2Post);
				PostHose(Event2Post);
				PostEventPrinter(Event2Post);
			}
		}
		
		// if start val is diff from last time and it's low, then send a START_BUTTON_DOWN event, otherwise send START_BUTTON_UP
		if ((currentButtonStates & START_MASK) != (lastButtonStates & START_MASK)) {
			if ((currentButtonStates & START_MASK) == 0) {
				Event2Post.EventType = START_BUTTON_DOWN;
				// ES_PostAll(Event2Post);
				PostGameplay(Event2Post);
				PostEventPrinter(Event2Post);
			} else {
				Event2Post.EventType = START_BUTTON_UP;
				// ES_PostAll(Event2Post);
				PostGameplay(Event2Post);
				PostEventPrinter(Event2Post);
			}
		}
		
		// if chem a is diff from last time and it's low, then send a CHEM_A_DOWN event, otherwise send CHEM_A_UP
		if ((currentButtonStates & CHEMA_MASK) != (lastButtonStates & CHEMA_MASK)) {
			Event2Post.EventParam = 0;
			if ((currentButtonStates & CHEMA_MASK) == 0) {
				Event2Post.EventType = CHEM_BUTTON_DOWN;
				// ES_PostAll(Event2Post);
				PostGameplay(Event2Post);
				PostEventPrinter(Event2Post);
			} else {
				Event2Post.EventType = CHEM_BUTTON_UP;
				// ES_PostAll(Event2Post);
				PostGameplay(Event2Post);
				PostEventPrinter(Event2Post);
			}
		}

		// if trigger is diff from last time and it's low, then send a CHEM_B_DOWN event, otherwise send CHEM_B_UP
		if ((currentButtonStates & CHEMB_MASK) != (lastButtonStates & CHEMB_MASK)) {
			Event2Post.EventParam = 1;
			if ((currentButtonStates & CHEMB_MASK) == 0) {
				Event2Post.EventType = CHEM_BUTTON_DOWN;
				// ES_PostAll(Event2Post);
				PostGameplay(Event2Post);
				PostEventPrinter(Event2Post);
			} else {
				Event2Post.EventType = CHEM_BUTTON_UP;
				// ES_PostAll(Event2Post);
				PostGameplay(Event2Post);
				PostEventPrinter(Event2Post);
			}
		}
			lastButtonStates = currentButtonStates;
	}

	return returnVal;
}
