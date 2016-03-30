#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "ES_Configure.h"
#include "ES_Types.h"


// typedefs for the stats
typedef enum  {
	INIT,
	ARMED,
	MIXING,
	SHOOTING,
	FAIL,
	WIN } GameplayState_t;

	

// public function prototypes
bool InitGameplay(uint8_t Priority);
bool PostGameplay(ES_Event ThisEvent);
ES_Event RunGameplay(ES_Event ThisEvent);
GameplayState_t QueryGameplay(void);

#endif //GAMEPLAY_H
