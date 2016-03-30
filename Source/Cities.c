// #define TEST

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
#include "IR_Pulse.h"

#include "Cities.h"
#include "Gameplay.h"
#include "EventPrinter.h"
#include <stdio.h>


/******************** Module Defines ***********************/
#define ALL_BITS (0xff<<2)

#define TOTAL_CITIES 5

// CHANGE whenever change these pins!!
#define CITIES_TIVA_PINMASK (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6)

#define NUM_HI_TO_SAVE 10

/******************** Private Functions ***********************/


/******************** Module Variables ***********************/
static uint8_t MyPriority;
static uint8_t lastIRStates;
static uint8_t citiesSaved[TOTAL_CITIES] = {0, 0, 0, 0, 0};
static uint8_t numCitiesSaved;
// CHANGE THIS WHENEVER TIVA PINS CHANGE
static uint8_t MaskCityNumber[TOTAL_CITIES] = {BIT6HI, BIT5HI, BIT4HI, BIT2HI, BIT3HI};

/******************** Module Code ***********************/
bool InitCities(uint8_t Priority) {

	MyPriority = Priority;
	// initialize Tiva hardware: Port A, pins 2,3,4,5,6

	//enable the clock to Port A by setting bit 0 in register
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_PRGPIO_R0;

	// wait for clock to be ready
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0)
	;

	// Connect Pin 2,3,4,5,6 to digital I/O port by writing to Digital Enable register
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= CITIES_TIVA_PINMASK;

	// Set data direction to input for these pins by writing them LOW to direction register
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) &= ~CITIES_TIVA_PINMASK;

	
	// read in initial values to be lastIRStates
	lastIRStates = HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) & CITIES_TIVA_PINMASK;
	// printf("Initial currentIRStates: %#03x\r\n",lastIRStates);

	// set citiesSaved all to 0
	resetCities();
	return true;
}

bool PostCities(ES_Event ThisEvent) {
	return ES_PostToService(MyPriority, ThisEvent);
}


ES_Event RunCities(ES_Event ThisEvent) {
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;

	// if detect IR signal at a city, and that city hasn't been saved yet
	if ((ThisEvent.EventType == CITY_IR_RECEIVED)	&& (citiesSaved[ThisEvent.EventParam] < NUM_HI_TO_SAVE)) {
		printf("City %u: %u out of %u HIs counted.\n\r",ThisEvent.EventParam, citiesSaved[ThisEvent.EventParam],NUM_HI_TO_SAVE);

		// then increment that city's counter
		citiesSaved[ThisEvent.EventParam]++;
		// if the city reached the number of high's necessary, send the CITY_SAVED event
		if (citiesSaved[ThisEvent.EventParam] == NUM_HI_TO_SAVE) {
			ES_Event Event2Post;
			Event2Post.EventType = CITY_SAVED;
			Event2Post.EventParam = ThisEvent.EventParam;
			printf("City %u saved", Event2Post.EventParam);
			// change to posting only to Gameplay after debugging
			PostGameplay(Event2Post);
			PostEventPrinter(Event2Post);
			// ES_PostAll(Event2Post);
			// remember that this city has been saved
			numCitiesSaved++;
			
			// if all cities have been saved, post ALL_CITIES_SAVED to gameplay
			if (numCitiesSaved == TOTAL_CITIES) {
				Event2Post.EventType = ALL_CITIES_SAVED;
				printf("All cities saved");
				PostGameplay(Event2Post);
				PostEventPrinter(Event2Post);
				// ES_PostAll(Event2Post);

			}
		}
	}
	return returnVal;
}


bool checkIREvents(void) {
 	// printf("I am running\r\n");
	bool returnVal = false;
	// get all values for ( == (lastIRStates && BIT0HI)currentIRStates && BIT0HI)
	uint8_t currentIRStates = HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) & CITIES_TIVA_PINMASK;

	//uint8_t currentIRStates;
	//uint8_t lastIRStates;
	
	// printf("currentIRStates: %#03x\r\n",currentIRStates);
	if (GetIRState() == true && currentIRStates != lastIRStates) {
		// printf("lastIRStates: %#03x\r\n",lastIRStates);
		//printf("currentIRStates: %#03x\r\n",currentIRStates);

		for (int i = 0; i < TOTAL_CITIES; i++) {
			// for a particular city, if currentIRStates differs from lastIRStates and (currentIRStates && BIT0HI) is high
			if ((currentIRStates & MaskCityNumber[i]) != (lastIRStates & MaskCityNumber[i])) {
				if ((currentIRStates & MaskCityNumber[i]) == MaskCityNumber[i]) {
					// then post CITY_IR_RECEIVED event to run with city # as the parameter
					ES_Event Event2Post;
					Event2Post.EventType = CITY_IR_RECEIVED;
					Event2Post.EventParam = i;
					
					PostCities(Event2Post);
					PostEventPrinter(Event2Post);
					// ES_PostAll(Event2Post);

					returnVal = true;
				}
			}
			
		}
		lastIRStates = currentIRStates;
	}

	return returnVal;
}

void resetCities(void) {
	for (int i = 0; i < TOTAL_CITIES; i++) {
		citiesSaved[i] = 0;
	}
	numCitiesSaved = 0;
}

// test harness
// goal: test Cities function

#ifdef TEST
#include "termio.h"

int main(void)
{
	
	TERMIO_Init();
	puts("\r\n In test harness for Cities: receiving IR signals (0)\r\n");
	InitCities(1);
	ES_Event ThisEvent;
	ThisEvent.EventType = ES_NO_EVENT;
	while (true) {
		RunCities(ThisEvent);
	}
	return 0;
}
#endif

