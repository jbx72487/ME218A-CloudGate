#ifndef CITIES_H
#define CITIES_H

#include "ES_Configure.h"
#include "ES_Types.h"



bool InitCities(uint8_t Priority);
bool PostCities(ES_Event ThisEvent);
ES_Event RunCities(ES_Event ThisEvent);

bool checkIREvents(void);
void resetCities(void);

#endif // CITIES_H
