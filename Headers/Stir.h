#ifndef STIR_H
#define STIR_H

#include "ES_Configure.h"
#include "ES_Types.h"

typedef enum  {
	STIR_INIT,
	STIR_IDLE,
	STIRRING} StirState_t;

bool InitStir(uint8_t Priority);
bool PostStir(ES_Event ThisEvent);
ES_Event RunStir(ES_Event ThisEvent);
StirState_t QueryStir(void);
bool CheckStirState(void);
	
#endif // STIR_H
	
