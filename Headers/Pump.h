#ifndef PUMP_H
#define PUMP_H

#include "ES_Configure.h"
#include "ES_Types.h"

typedef enum  {
	INIT_PUMP,
	PUMP_IDLE,
	GOING_UP,
	GOING_DOWN} PumpState_t;


bool InitPump(uint8_t Priority);
bool PostPump(ES_Event ThisEvent);
ES_Event RunPump(ES_Event ThisEvent);
PumpState_t QueryPump(void);
bool CheckPumpLevel(void);
	
#endif // PUMP_H
