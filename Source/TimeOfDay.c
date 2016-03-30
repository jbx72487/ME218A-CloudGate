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

#include "ServoControl.h"

#include "TimeOfDay.h"
#include <stdio.h>

/******************** Module Defines ***********************/

// #define TIMESERVO_TIVA_PINMASK (GPIO_PIN_4)

#define ALL_BITS (0xff<<2)

#define ONE_SEC 976
#define GAME_TIME 60*ONE_SEC

// angles & timing for servo
#define NUM_INCREMS 20
#define INCREM_TIME GAME_TIME / NUM_INCREMS

#define STARTING_ANG 10
#define INCREM_ANGLE ((int) 150/NUM_INCREMS)


/******************** Private Functions ***********************/

/******************** Module Variables ***********************/
static uint8_t MyPriority;
static TimeOfDayState_t currentState;
static uint8_t currentAngle;

/******************** Module Code ***********************/
bool InitTimeOfDay(uint8_t Priority) {

	MyPriority = Priority;

	/*
	//enable the clock to Port B by setting correct bit in register
	HWREG(SYSCTL_RCGCGPIO) |= BIT1HI;

	// wait for clock to be ready
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != BIT1HI)
	;

	// Connect Pin to digital I/O port by writing to Digital Enable register
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= TIMESERVO_TIVA_PINMASK;

	// Set data direction to output for these pins by writing them HIGH to direction register
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= TIMESERVO_TIVA_PINMASK;

	// set initial values to be low
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~TIMESERVO_TIVA_PINMASK;

	// initialize servo
	ServoInit();
	*/
	
	// set currentState to INIT
	currentState = TIME_INIT;

	ES_Event ThisEvent;
	ThisEvent.EventType = ES_INIT;

	// post ES_INIT to itself
	if (ES_PostToService(MyPriority, ThisEvent) == true)
	{
		return true;
	} else {
		return false;
	}
}

bool PostTimeOfDay(ES_Event ThisEvent) {
	if (ES_PostToService(MyPriority, ThisEvent) == true)
		return true;
	else
		return false;
}


ES_Event RunTimeOfDay(ES_Event ThisEvent) {

	// printf("In RunTimeOfDay\n\r");
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;

	TimeOfDayState_t nextState = currentState;

	switch (currentState) {
		case TIME_INIT:
			// printf("TimeOfDay: TIME_INIT state, %u event\r\n",ThisEvent.EventType);
			if (ThisEvent.EventType == ES_INIT) {
				// printf("TimeOfDay: Received ES_INIT event");
				nextState = TIME_ZERO;
				// set servo to initial angle
				currentAngle = STARTING_ANG;
				SetServoAngle(currentAngle, TIME_OF_DAY);
			}
			break;
		case TIME_ZERO:
			// printf("TimeOfDay: TIME_ZERO state\r\n");

			if (ThisEvent.EventType == GAME_STARTED) {
				// move to incrementing state
				nextState = TIME_INCREMENTING;
				// start servo timer
				ES_Timer_InitTimer(TIME_OF_DAY_TIMER, INCREM_TIME);
			}
			break;
		case TIME_INCREMENTING:
			// printf("TimeOfDay: TIME_INCREMENTING state\r\n");
			// if TIME_OF_DAY_TIMER runs out
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == TIME_OF_DAY_TIMER) {
				printf("TimeOfDay: ES_TIMEOUT. Servo moving now \r\n");
				// increment servo position
				currentAngle += INCREM_ANGLE;
				SetServoAngle(currentAngle,TIME_OF_DAY);
				// start servo timer
				ES_Timer_InitTimer(TIME_OF_DAY_TIMER, INCREM_TIME);
			}
			// if game is stopped, stop the servo
			else if (ThisEvent.EventType == GAME_ENDED) {
				// go to TIME_STOPPED
				printf("TimeOfDay: GAME_RESET\r\n");
				nextState = TIME_ZERO;
				printf("Time stopped\r\n");
				// set servo to initial angle
				currentAngle = STARTING_ANG;
				SetServoAngle(currentAngle, TIME_OF_DAY);
			}
			break;
	}
	currentState = nextState;
	return returnVal;

}
