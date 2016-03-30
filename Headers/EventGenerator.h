#ifndef EVENT_GENERATOR_H
#define EVENT_GENERATOR_H

#include "ES_Configure.h"
#include "ES_Types.h"

bool InitEventGenerator(uint8_t Priority);
bool PostEventGenerator(ES_Event ThisEvent);
ES_Event RunEventGenerator(ES_Event ThisEvent);

#endif // EVENT_GENERATOR_H
