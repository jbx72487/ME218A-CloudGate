#ifndef HOSE_H
#define HOSE_H

#include "ES_Configure.h"
#include "ES_Types.h"

typedef enum {HOSE_INIT, HOSE_IDLE, HOSE_SHOOTING} HoseState_t;

bool InitHose(uint8_t Priority);
bool PostHose(ES_Event ThisEvent);
ES_Event RunHose(ES_Event ThisEvent);

#endif // HOSE_H
