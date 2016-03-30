#ifndef IR_PULSE_H
#define IR_PULSE_H

#include "ES_Configure.h"
#include "ES_Types.h"

#define PULSE_CHANNEL 4

// Initializes servo pins. Call once before setting angle
void PulseInit(void);

void TurnIROn(void);
void TurnIROff(void);
bool GetIRState(void);
#endif // IR_PULSE_H
