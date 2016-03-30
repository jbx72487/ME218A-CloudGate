// #define LED_TEST

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

#include "LEDControl.h"
#include "LEDShiftRegister.h"

#include <stdio.h>

/******************** Module Defines ***********************/
#define ALL_BITS (0xff<<2)
#define FLASH_TIME 200




/******************** Private Functions ***********************/

/******************** Module Variables ***********************/
static uint8_t MyPriority;
static uint32_t currentLEDVals;
static uint32_t flashingLEDs;
static LEDMode_t currentFlashState;

/******************** Module Code ***********************/
bool InitLEDControl(uint8_t Priority) {

	LED_SR_Init();
	MyPriority = Priority;

	// start currentFlashState as off
	currentFlashState = OFF;
	// clear currentLEDVals
	currentLEDVals = 0;
	// clear flashingLEDs
	flashingLEDs = 0;
	// start flash timer
	ES_Timer_InitTimer(LED_FLASH_TIMER, FLASH_TIME);

	return true;
}

bool PostLEDControl(ES_Event ThisEvent) {
	if (ES_PostToService(MyPriority, ThisEvent) == true)
		return true;
	else
		return false;
}


ES_Event RunLEDControl(ES_Event ThisEvent) {
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;

	// if the flash timer timed out
	if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == LED_FLASH_TIMER) {
		// printf("LED_FLASH_TIMER expired\r\n");
		
		// if currentFlashState is ON, use bitmask to turn flashing LEDs off
		if (currentFlashState == ON) {
			//printf("On");
			currentLEDVals &= ~flashingLEDs;
			currentFlashState = OFF;
			// printf("currentLEDVals: %#10x\r\n",currentLEDVals);
		}
		// if currentFlashState is OFF, use bitmask to turn flashing LEDs on
		else {
			// printf("Off"); 
			currentLEDVals |= flashingLEDs;
			currentFlashState = ON;
			// printf("currentLEDVals: %#10x\r\n",currentLEDVals);
		}
		// toggle the currentFlashState & SR_Write to the shift register in either case
		LED_SR_Write(currentLEDVals);

		// restart the timer
		ES_Timer_InitTimer(LED_FLASH_TIMER, FLASH_TIME);
	}
	// printf("CurrentLEDVals: %#10x\r\n",LED_SR_GetCurrentRegister());
	return returnVal;
}

void setLED(uint32_t whichLED, LEDMode_t mode) {
	// takes in which LED to control and what to do with it
	// if duty cycle is ON
	if (mode == ON || mode == CITY_OFF) {
		// OR that bitmask to current shift register value to turn those on
		currentLEDVals |= whichLED;
		// clear those bits from the flashing bitmask
		flashingLEDs &= ~(whichLED);
	// if duty cycle is OFF
	} else if (mode == OFF || mode == CITY_ON) {
		// AND ~bitmask to current shift register value to turn those off
		currentLEDVals &= ~(whichLED);
		// clear those bits from the flashing bitmask
		flashingLEDs &= ~(whichLED);
	} else {// if duty cycle is FLASHING 
		// join that with the FLASHING bitmask
		flashingLEDs |= whichLED;
	}
	// SR_Write to the shift register		
	LED_SR_Write(currentLEDVals);
	// printf("Attempting to set %#10x to %u mode\r\n", whichLED, mode);
	// printf("CurrentLEDVals: %#10x\r\n",currentLEDVals);
	// printf("Currently flashing LEDs: %#10x\r\n", flashingLEDs);

}


// test harness
// goal: test Cities function

#ifdef LED_TEST
#include "termio.h"

int main(void)
{
	PortFunctionInit();
	TERMIO_Init();
	puts("\r\n In test harness for LEDControl: All LEDs should be flashing, except for pin 31 which is flashing and pin 30 which is on (0)\r\n");
	InitLEDControl(1);

	// setLED(LED_ALL, OFF);

	/*char input = getchar();
	
	while (input != '0') {
		
		//setLED(LED_ALL,ON);
		//setLED(LED_SD_BLUE,OFF);
		setLED(LED_ALL, ON);

		setLED(LED_SACRAMENTO_RED, ON);
		setLED(LED_SACRAMENTO_BLUE, OFF);

		//setLED(LED_LA_RED, ON);
		printf("Blue Off\r\n");
		input = getchar();
		setLED(LED_SACRAMENTO_RED,OFF);
		setLED(LED_SACRAMENTO_BLUE, ON);
		//setLED(LED_LA_RED, OFF);
		printf("Blue On\r\n");
		*/
		/*
		if (input >= 'a' && input <='z') {
			setLED(LED_ALL_INDIV, OFF);
			setLED(LED_ALL_CITIES, CITY_OFF);
			setLED(BIT0HI << (input - 'a'), ON);
			printf("Shift Register value: %u\r\n",LED_SR_GetCurrentRegister());
		}
		input = getchar();
		}
		*/
		setLED(LED_ALL_INDIV, OFF);
		setLED(LED_ALL_CITIES_RED, CITY_OFF);
		setLED(LED_ALL_CITIES_BLUE, CITY_ON);
	
	
	return 0;
}
#endif



