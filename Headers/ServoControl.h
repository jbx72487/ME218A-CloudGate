#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "ES_Configure.h"
#include "ES_Types.h"

#define LEFT_FLAG 0
#define RIGHT_FLAG 3
#define TIME_OF_DAY 2

// Initializes servo pins. Call once before setting angle
void ServoInit(void);

// Sets specified servo angle, 0-170 degree range of motion
bool SetServoAngle(uint8_t Angle, uint8_t ServoID);

// Toggles the position of the flags from active to inactive
void SetFlagsActive(void);
void SetFlagsInactive(void);
void ToggleFlags(void);


#endif // SERVO_CONTROL_H
