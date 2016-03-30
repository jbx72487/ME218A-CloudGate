#ifndef TIME_OF_DAY_H
#define TIME_OF_DAY_H

#include "ES_Configure.h"
#include "ES_Types.h"


typedef enum {TIME_INIT, TIME_ZERO, TIME_INCREMENTING} TimeOfDayState_t;

bool InitTimeOfDay(uint8_t Priority);
bool PostTimeOfDay(ES_Event ThisEvent);
ES_Event RunTimeOfDay(ES_Event ThisEvent);

#endif // TIME_OF_DAY_H
