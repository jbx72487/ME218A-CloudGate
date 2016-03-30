#ifndef BUTTONS_H
#define BUTTONS_H

#include "ES_Configure.h"
#include "ES_Types.h"



bool InitButtons(uint8_t Priority);
bool PostButtons(ES_Event ThisEvent);
ES_Event RunButtons(ES_Event ThisEvent);

bool checkButtonEvents(void);

#endif // BUTTONS_H
