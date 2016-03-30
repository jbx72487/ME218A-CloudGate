//#define SERVO_TEST
/****************************************************************************
 Module
   ServoControl.c

 Revision
   1.0.0

 Description
   This is the service for interfacing with the servos

 Notes

 History
 When           	Who     What/Why
 -------------- 	---     --------
 11/08/15	19:07		JAM			debugging on uVision
 11/08/15 02:58 	JAM			first pass translating pseudo-code
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
 
 #include "ServoControl.h"
 #include "PWM_Control.h"

#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

/*----------------------------- Module Defines ----------------------------*/
 #define PERIOD 25000
 
 #define TOD_MIN_POSITION 0.03*PERIOD
 #define TOD_MAX_POSITION 0.12*PERIOD
 #define TOD_RANGE (TOD_MAX_POSITION - TOD_MIN_POSITION)
 
 #define LEFT_MIN_POSITION 0.035*PERIOD
 #define LEFT_MAX_POSITION 0.13*PERIOD
 #define LEFT_RANGE (LEFT_MAX_POSITION - LEFT_MIN_POSITION)
 
 #define RIGHT_MIN_POSITION 0.035*PERIOD
 #define RIGHT_MAX_POSITION 0.13*PERIOD
 #define RIGHT_RANGE (RIGHT_MAX_POSITION - RIGHT_MIN_POSITION)

/*---------------------------- Module Functions ---------------------------*/
static void ToggleLeftFlag(void);
static void ToggleRightFlag(void);

/*---------------------------- Module Variables ---------------------------*/


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     ServoInit

 Parameters
     none

 Returns
     none

 Description
     Initializes the servo output pins and sets the initial positions.
 Notes

 Authors
	Jordan A. Miller,	11/08/15,	14:52
****************************************************************************/
void ServoInit(void) {
	// Call init function for analog pin associated with rotary pot
	//InitializeServoPins();
	PWMInit();
	PWM8_TIVA_SetPeriod(PERIOD,0);	// Group 0 includes PB6 and PB7
	PWM8_TIVA_SetPeriod(PERIOD,1);	// Group 1 includes PB4 and PB5

	// Set the initial starting position of the servos
	SetFlagsInactive();
	//SetServoAngle(90, LEFT_FLAG);
	//SetServoAngle(90, RIGHT_FLAG);
	
}

/****************************************************************************
 Function
     SetServoAngle

 Parameters
     uint8_t : the desired angle, in degrees
     uint8_t : the ID of the servo to move

 Returns
     bool, false if inputs are invalid, true otherwise

 Description
     Sets the servo to the desired angle position
 Notes

 Authors
	Jordan A. Miller,	11/08/15,	15:12
****************************************************************************/
bool SetServoAngle(uint8_t angle, uint8_t servoID) {
	// Make sure input is valid
	if (angle > 180) {
		printf("%u degrees is more than the 170 degrees maximum \r\n",angle);
		return false;
	}

	if (servoID != LEFT_FLAG && servoID != RIGHT_FLAG && servoID != TIME_OF_DAY) {
		printf("servoID is invalid");
		return false;
	}
		
	uint32_t conversion;
	switch (servoID) {
		case TIME_OF_DAY:
			// Convert angle into the range of motion of the servo
			conversion = TOD_MIN_POSITION + (angle/180.0)*TOD_RANGE;
		break;
		
		case LEFT_FLAG:
			// Convert angle into the range of motion of the servo
			conversion = LEFT_MIN_POSITION + (angle/180.0)*LEFT_RANGE;
		break;
		
		case RIGHT_FLAG:
			// Convert angle into the range of motion of the servo
			conversion = RIGHT_MIN_POSITION + (angle/180.0)*RIGHT_RANGE;
		break;
	}
	
	PWM8_TIVA_SetPulseWidth(conversion,servoID);
	printf("Pulse Width: %u\n\r",conversion);

	return true;
}

void SetFlagsActive(void) {
	printf("Flags Activated\r\n");
	SetServoAngle(55,LEFT_FLAG);
	SetServoAngle(135,RIGHT_FLAG);
}

void SetFlagsInactive(void) {
	printf("Flags Inactivated\r\n");
	SetServoAngle(145,LEFT_FLAG);
	SetServoAngle(45,RIGHT_FLAG);
}

void ToggleFlags(void) {
	//static bool flags_active;
	
	ToggleLeftFlag();
	ToggleRightFlag();
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void ToggleLeftFlag(void) {
	uint8_t angle1 = 55;
	uint8_t angle2 = 145;
	
	static uint8_t left_angle;
	
	if (left_angle == angle1)
		left_angle = angle2;
	else
		left_angle = angle1;
		
	SetServoAngle(left_angle,LEFT_FLAG);
	printf("Left Flag at angle of %u degrees\r\n", left_angle);
	
}

static void ToggleRightFlag(void) {
	uint8_t angle1 = 135;
	uint8_t angle2 = 45;
	
	static uint8_t right_angle;

	if (right_angle == angle1)
		right_angle = angle2;
	else
		right_angle = angle1;
	
	SetServoAngle(right_angle,RIGHT_FLAG);
	printf("Right Flag at angle of %u degrees\r\n",right_angle);
	
}


	
#ifdef SERVO_TEST
#include "EnablePA25_PB23_PD7_PF0.h"

int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
			| SYSCTL_XTAL_16MHZ);
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
	
	ServoInit();
	
	getchar();
	
	SetServoAngle(120,TIME_OF_DAY);
	//SetServoAngle(80,RIGHT_FLAG);
	//SetServoAngle(0,TIME_OF_DAY);
//	
	// Channel 0 is PB6
	//PWM_TIVA_Init();
	//PWM_TIVA_SetPeriod(10000,0);
	//PWM_TIVA_SetDuty(7,0);
	
	uint8_t angle = 0;
	while (true) {
		
		char input = getchar();
		if (input == 'a') {
			break;
		}
		else if (angle > 180) {
			break;
		}
		SetServoAngle(angle,TIME_OF_DAY);
		SetFlagsInactive();
		
		//PWM_TIVA_SetDuty(counter,0);
		printf("Angle: %u\n\n\r",angle);
		angle+=5;
		
		input = getchar();
		if (input == 'a') {
			break;
		}
		else if (angle > 180) {
			break;
		}
		SetServoAngle(angle,TIME_OF_DAY);
		SetFlagsActive();
		
		//PWM_TIVA_SetDuty(counter,0);
		printf("Angle: %u\n\n\r",angle);
		angle+=5;
	}
		
	//PWM8_TIVA_SetDuty(7,TIME_OF_DAY);
	SetServoAngle(90,TIME_OF_DAY);
	
	
	return 0;
}

#endif

	
