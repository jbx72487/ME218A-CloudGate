// ****************** #includes *******************/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "Gameplay.h"
#include "LEDControl.h"
#include "EnablePA25_PB23_PD7_PF0.h"
#include "ServoControl.h"
#include "AudioControl.h"
#include "Stir.h"
#include "ExternalLine.h"
#include "Cities.h"
#include "EventPrinter.h"
#include "TimeOfDay.h"
#include "IR_Pulse.h"


// ****************** #defines *******************/
// how long before users loses by timeout

// these times assume a 1.000mS/tick timing
#define ONE_SEC 976

#define GAME_TIME 60*ONE_SEC

// how long user can be inactive before automatically lose
#define INACTION_TIME 20*ONE_SEC

// how many LEDs worth of chems the user must mix/shoot
#define TOTAL_AMMO 6 // cannot easily change this number... don't do it
#define CHEMS_PER_AMMO 2 // cannot easily change this number... don't do it


// set how long the winning and losing displays last
#define WIN_DISPLAY_TIME 30*ONE_SEC
#define FAIL_DISPLAY_TIME 30*ONE_SEC


// angles for servo
#define FLAG_ACTIVE_ANG 120
#define FLAG_INACTIVE_ANG 60

// ****************** Module functions *******************/
void startArmedUX(void);
void stopArmedUX(void);
void startMixingUX(void);
void addChemUX(void);
void addChemLightUX(void);
void nextChemUX(void);
void wrongChemAddedUX (void);
void stopMixingUX(void);
void startSprayingUX(void);
void fireOnUX(void);
void fireReadyUX(void);
void fireOffUX(void);
void usedOneAmmoUX(void);
void citySavedUX(uint8_t whichCity);
void stopSprayingUX(void);
void startWinUX(void);
void stopWinUX(void);
void startFailUX(void);
void stopFailUX(void);
void resetGameplay(void);
void resetUX(void);

// ****************** Module variables *******************/

static GameplayState_t currentState;
static uint8_t numChems;
static uint8_t MyPriority;
// static uint8_t lastChem;
static uint8_t thisChem;
static uint8_t numAmmo;


bool InitGameplay(uint8_t Priority) {
	MyPriority = Priority;

	// initialize servo
	ServoInit();
	
	SetFlagsInactive();
	
	// initialize audio
	AudioControlInit();
	ExternalLineInit();
	// set currentState to INIT
	currentState = INIT;
	

	ES_Event ThisEvent;
	ThisEvent.EventType = ES_INIT;

	// post ES_INIT to itself
	if (ES_PostToService(MyPriority, ThisEvent) == true)
	{
		return true;
	} else {
		return false;
	}
}

bool PostGameplay(ES_Event ThisEvent) {
	return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunGameplay(ES_Event ThisEvent) {
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;

	// Initialize nextState as current state
	GameplayState_t nextState = currentState;

	switch (currentState) {

		case INIT:
			if (ThisEvent.EventType == ES_INIT) {
				resetGameplay();
				nextState = ARMED;
				startArmedUX();
			}
			break;

		case ARMED:
			if (ThisEvent.EventType == START_BUTTON_UP) {
				printf("Game Started\r\n");
				// start timers
				ES_Timer_InitTimer(GAME_TIMER, GAME_TIME);
				ES_Timer_InitTimer(INACTION_TIMER, INACTION_TIME);

				// send GAME_STARTED event to TimeOfDaySM
				ES_Event Event2Post;
				Event2Post.EventType = GAME_STARTED;
				// ES_PostAll(Event2Post); // post to TimeOfDay 
				PostTimeOfDay(Event2Post);
				PostEventPrinter(Event2Post);

				stopArmedUX();
				startMixingUX();
				nextChemUX();
				nextState = MIXING;
			}  
			break;
	
		case MIXING:
			switch (ThisEvent.EventType) {
				case CHEM_BUTTON_DOWN:
					if (QueryStir() == STIRRING  && numChems < TOTAL_AMMO * CHEMS_PER_AMMO) {
						if (thisChem == ThisEvent.EventParam) {
							numChems++;
							printf("Correct button press. numChems: %u\r\n", numChems);
							addChemUX();
							if (numChems % CHEMS_PER_AMMO == 0) {
								numAmmo++;
								addChemLightUX();
							}
							if (numChems == TOTAL_AMMO * CHEMS_PER_AMMO) {
								stopMixingUX();
								startSprayingUX();
								nextState = SHOOTING;
								printf("Mixing complete");
							} else {
								nextChemUX();
								printf("Next chem UX");
							}
						} else {
							// if pressed the wrong chemical button
							if (numChems > CHEMS_PER_AMMO)
								numChems -= CHEMS_PER_AMMO;
							else
								numChems = 0;
							
							if (numAmmo > 0)
								numAmmo--;
							// play the sound for wrong chemical added and decrement a light if necessary
							wrongChemAddedUX();
						}
					}
					break;
				case CHEM_BUTTON_UP:
					ES_Timer_InitTimer(INACTION_TIMER, INACTION_TIME);
					break;
				case ES_TIMEOUT:
					if (ThisEvent.EventParam == INACTION_TIMER || ThisEvent.EventParam == GAME_TIMER) {
						
						// send GAME_ENDED event to TimeOfDaySM
						ES_Event Event2Post;
						Event2Post.EventType = GAME_ENDED;
						// ES_PostAll(Event2Post); // only post to TimeOfDay 
						PostTimeOfDay(Event2Post);
						PostEventPrinter(Event2Post);
						
						stopMixingUX();
						startFailUX();
						nextState = FAIL;
						ES_Timer_InitTimer(GAME_STATE_TIMER,FAIL_DISPLAY_TIME);
					}
					break;
			}
			break;
		case SHOOTING:
			switch (ThisEvent.EventType) {
				ES_Event Event2Post;
				case FIRE_READY:
					ES_Timer_InitTimer(INACTION_TIMER, INACTION_TIME);
					fireReadyUX();
					break;
				case FIRE_ON:
					ES_Timer_InitTimer(INACTION_TIMER, INACTION_TIME);
					// UX: Flash indicator light & turn on IR emitter
					fireOnUX();
					break;
				case FIRE_OFF:
					ES_Timer_InitTimer(INACTION_TIMER, INACTION_TIME);
					// UX: Turn off IR emitter & indicator light
					fireOffUX();
					break;
				case CITY_SAVED:
					// UX: Turn that city’s LED blue
					citySavedUX(ThisEvent.EventParam);
					break;
				case ALL_CITIES_SAVED:
					ES_Timer_InitTimer(GAME_STATE_TIMER,WIN_DISPLAY_TIME);
						
					// send GAME_ENDED event to TimeOfDaySM
					Event2Post.EventType = GAME_ENDED;
					// ES_PostAll(Event2Post); // only post to TimeOfDay 
					PostTimeOfDay(Event2Post);
					PostEventPrinter(Event2Post);

					startWinUX();
					nextState = WIN;
					break;
				case USED_ONE_AMMO:
					numAmmo--;
					usedOneAmmoUX();	
					if (numAmmo>0) {
							printf("numAmmo: %u\r\n", numAmmo); // FOR DEBUGGING
							
						} else {

							// send GAME_ENDED event to TimeOfDaySM
							ES_Event Event2Post;
							Event2Post.EventType = GAME_ENDED;
							// ES_PostAll(Event2Post); //  only post to TimeOfDay 
							PostTimeOfDay(Event2Post);
							PostEventPrinter(Event2Post);

							stopSprayingUX();
							startFailUX();
							nextState = FAIL;
							ES_Timer_InitTimer(GAME_STATE_TIMER, FAIL_DISPLAY_TIME);
						}
						break;

				case ES_TIMEOUT:
					if (ThisEvent.EventParam == INACTION_TIMER || ThisEvent.EventParam == GAME_TIMER) {
						// send GAME_ENDED event to TimeOfDaySM
						ES_Event Event2Post;
						Event2Post.EventType = GAME_ENDED;
						// ES_PostAll(Event2Post); //  only post to TimeOfDay 
						PostTimeOfDay(Event2Post);
						PostEventPrinter(Event2Post);
						stopSprayingUX();
						startFailUX();
						nextState = FAIL;
						ES_Timer_InitTimer(GAME_STATE_TIMER, FAIL_DISPLAY_TIME);
					}
					break;
			}
			break;

		case FAIL:
			if ((ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == GAME_STATE_TIMER) || ThisEvent.EventType == START_BUTTON_UP) {
					// send GAME_ENDED event to TimeOfDaySM
					ES_Event Event2Post;
					Event2Post.EventType = GAME_ENDED;
					// ES_PostAll(Event2Post); // only post to TimeOfDay 
					PostTimeOfDay(Event2Post);
					PostEventPrinter(Event2Post);

					// reset software & hardware
					resetGameplay();
					nextState = ARMED;
					startArmedUX();
			}
			break;
		case WIN:
			if ((ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == GAME_STATE_TIMER) || ThisEvent.EventType == START_BUTTON_UP) {

					// send GAME_ENDED event to TimeOfDaySM
					ES_Event Event2Post;
					Event2Post.EventType = GAME_ENDED;
					// ES_PostAll(Event2Post); // only post to TimeOfDay 
					PostTimeOfDay(Event2Post);
					PostEventPrinter(Event2Post);

					// reset software & hardware & next state is ARMED
					resetGameplay();
					nextState = ARMED;
					startArmedUX();
			}
			break;
	}
	// set currentState to nextState
	currentState = nextState;

	return returnVal;
}



GameplayState_t QueryGameplay(void) {
	return(currentState);
}



void startArmedUX(void) {
	printf("Started Armed UX: Deploy flags, start flashing all LEDs\r\n");
	// deploy flags
	//SetFlagsActive();

	// flash all LEDs
	setLED(LED_ALL_INDIV,FLASHING);
	setLED(LED_ALL_CITIES_BLUE,CITY_OFF);
	setLED(LED_ALL_CITIES_RED,CITY_ON);

	// play starting sound
	playSound(AUDIO_INTRO);
}

// stopArmedUX - Return flags, Turn off all LEDs momentarily, start pulsing red LEDs, BONUS: pulse red LEDs around CA locations with low frequency (slowly increasing frequency over the duration of the game)
void stopArmedUX(void) {
	printf("Stopped Armed UX: Return flags, Turn off all LEDs momentarily, start pulsing red LEDs\r\n");
	// return flags
//	SetServoAngle(FLAG_INACTIVE_ANG, LEFT_FLAG);
//	SetServoAngle(FLAG_INACTIVE_ANG, RIGHT_FLAG);
	SetFlagsActive();
	
	// turn off all LEDs momentarily
	setLED(LED_ALL_INDIV, OFF);
	/*
	// turn on red LEDs
		setLED(LED_ALL_CITIES, CITY_OFF);
	setLED( LED_ALL_CITIES_RED , CITY_ON);
*/
	// stop sound
	stopAllSounds();
}

//startMixingUX - Turn on ‘MIX CHEMICALS’ backlight, Turn on ARROW on cylinder, BONUS: Flash these 3x and HOLD
void startMixingUX(void) {
	printf("Started Mixing UX: Turn on ‘MIX CHEMICALS’ backlight, Turn on ARROW on cylinder\r\n");

	setLED( (LED_BACK_MIX | LED_BACK_ARROW), FLASHING);
	// tell user to add and mix chemicals
	playSound(AUDIO_INSTRUCTION_MIX);

}

void nextChemUX(void) {
	printf("Next Chem UX: turn on light from next chem\r\n");
	// lastChem = thisChem;
	thisChem = rand() % 2;
	if (thisChem == 0) {
		// if value in array is 0, turn on yellow
		setLED(LED_YELLOW_CHEM, FLASHING);
	} else {
		// if value in array is 1, turn on blue
		setLED(LED_BLUE_CHEM, FLASHING);
	}
}

void addChemUX(void) {
	printf("Added ChemUX: turn off light from previous chem & light up a green chem on canister, play burbling sound \r\n");
	if (thisChem == 0) {
		// if prev value in array is 0, turn off yellow
		setLED(LED_YELLOW_CHEM, OFF);
	} else {
		// if prev value in array is 1, turn off blue
		setLED(LED_BLUE_CHEM, OFF);
	}
	// play chemical burbling sound for each successful chem unit created
	playSound(AUDIO_LIQUID);
}

void addChemLightUX(void) {
	// increment one green LED for each successful sequence of alternating buttons
	setLED( LED_CANISTER1 << (numAmmo-1), ON);

}

void wrongChemAddedUX (void) {
	// play the sound for wrong chemical added and decrement a light if necessary
	playSound(AUDIO_WRONG_BUTTON);
	setLED(LED_CANISTER1 << (numAmmo), OFF);

}

// stopMixingUX - turn off ‘MIX CHEMICALS’ backlight and turn off ARROW backlight
void stopMixingUX(void) {
	printf("Stop Mixing UX: turn off ‘MIX CHEMICALS’ backlight and turn off ARROW backlight\r\n");
	setLED( (LED_BACK_MIX | LED_BACK_ARROW), OFF);
	// stop all sounds
	stopAllSounds();
}

// startSprayingUX - Turn on ‘PUMP’ message, turn on LEDs at end of hose, Flash LEDs at the end of hose each time a chem unit is fired
void startSprayingUX(void) {
	printf("Started Shooting UX: Turn on ‘PUMP’ message, turn on 'SHOOT'/hose backlight\r\n");
	setLED( (LED_BACK_PUMP | LED_BACK_SPRAY), FLASHING);
	// tell user to pump and spray
	playSound(AUDIO_INSTRUCTION_SPRAY);
	// flash first ammo
	// setLED( LED_CANISTER1 << (numAmmo-1), FLASHING);
}

// fireOnUX: turn on IR led & flash indicator light
void fireOnUX(void) {
	printf("Fire ON UX: turn on IR led & flash indicator light\r\n");
	/*
	ES_Event Event2Post;
	Event2Post.EventType = TURN_IR_ON;
	// ES_PostAll(Event2Post); // only post to IRControl
	PostIRControl(Event2Post);
	PostEventPrinter(Event2Post);
*/
	// flash indicator light
	printf("Make raindrops and ammo in use flash");
	setLED( LED_SHOOTING_INDICATOR, FLASHING);
	setLED( LED_CANISTER1 << (numAmmo-1), FLASHING);
	// play spraying sound
	playSound(AUDIO_SPRAYING);
}

// fireReadyUX: turn off IR LED & turn on indicator light
void fireReadyUX(void) {
	printf("Fire READY UX: turn raindrops on & turn ammo on\r\n");
	/*
	ES_Event Event2Post;
	Event2Post.EventType = TURN_IR_OFF;
	// ES_PostAll(Event2Post); // only post to IRControl
	PostIRControl(Event2Post);
	PostEventPrinter(Event2Post);
*/
	// turn on indicator light
	setLED( LED_SHOOTING_INDICATOR, ON);
	setLED( LED_CANISTER1 << (numAmmo-1), ON);
	// stop playing spray sound
	stopAllSounds();
}

// fireOffUX: turn off IR LED & turn off indicator light
void fireOffUX(void) {
	printf("Fire OFF UX: turn raindrops off & turn ammo on\r\n");
	/*
	ES_Event Event2Post;
	Event2Post.EventType = TURN_IR_OFF;
	// ES_PostAll(Event2Post); // only post to IRControl
	PostIRControl(Event2Post);
	PostEventPrinter(Event2Post);
	*/
	
	// turn off indicator light
	printf("Turn off raindrops & turn ammo on");
	setLED( LED_SHOOTING_INDICATOR, OFF);
	setLED( LED_CANISTER1 << (numAmmo-1), ON);
	// stop playing spray sound
	stopAllSounds();

}


void usedOneAmmoUX(void) {
	printf("Used One Ammo UX: play cannon/tank shot when chem unit is fired, turn off topmost green LED\r\n");
	// turn off topmost green LED
	setLED(LED_CANISTER1 << (numAmmo), OFF);
	setLED( LED_CANISTER1 << (numAmmo-1), FLASHING);
}

// CITY_SAVED event UX - play cannon/tank shot when chem unit is fired, turn the city’s LED ring from red to blue and HOLD, play thunderstorm audio when a city is extinguished, turn off topmost green LED
void citySavedUX(uint8_t whichCity) {
	printf("City Saved UX: turn the city’s LED ring from red to blue and HOLD, play thunderstorm audio when a city is extinguished\r\n");
	// Turn off that city’s red LEDs ring
	setLED(LED_FIRST_RED << (whichCity), CITY_OFF);
	// Turn on that city’s blue LEDs ring
	setLED(LED_FIRST_BLUE<< (whichCity), CITY_ON);
	// play thunderstorm audio
	playSound(AUDIO_THUNDERSTORM);
}


// stopSprayingUX - turn off end of hose LEDs, turn off ‘PUMP’ message
void stopSprayingUX(void) {
	printf("Stopped Shooting UX: turn off end of hose LEDs, turn off ‘PUMP’ message\r\n");
	setLED( (LED_BACK_PUMP | LED_BACK_SPRAY), OFF);
	stopAllSounds();
}

void startWinUX(void) {
	printf("Started Win UX: flash all blue LEDs, play triumphant music, deploy flags\r\n");
	// startWinUX - flash all blue LEDs with low frequency, play triumphant music, flap flags
	
	// flash all blue LEDs
	setLED(LED_ALL_CITIES_BLUE, FLASHING);
	setLED(LED_ALL_CITIES_RED, CITY_OFF);
	setLED(LED_ALL_INDIV, OFF);
	// play triumphant music
	playSound(AUDIO_WINNING);
	
	// deploy flags
	//SetFlagsActive();
	
	// set external line to 5V
	setExternalLineHi();
}

void startFailUX(void) {
	printf("Started Fail UX: flash all red LEDs, play WOMP WOMP WOMP\r\n");
	// SetFlagsInactive();
	
	// startFailUX - flash all red LEDs with max frequency, play WOMP WOMP WOMP
	setLED(LED_ALL_CITIES_RED, FLASHING); 
	setLED(LED_ALL_CITIES_BLUE, CITY_OFF);
	setLED(LED_ALL_INDIV, OFF);
	// play womp womp
	playSound(AUDIO_LOSING);
}



void resetGameplay(void) {
	printf("Reset game\r\n");
	
	// reset button pressing

	// undo any chemicals mixed
	numChems = 0;
	numAmmo = 0;
		
	// undo any cities saved
	resetCities();
	
	// undo effects of any victory/defeat
	setExternalLineLo();

	// reset all the lights, sounds, & servos
	resetUX();
}

void resetUX(void) {
	// set all LEDs to off
	setLED(LED_ALL_INDIV,OFF);
	setLED(LED_ALL_CITIES,CITY_OFF);
	setLED(LED_START_BUTTON,FLASHING);
	
	// set all audio to off
	stopAllSounds();
	
	// set all servos to off
	SetFlagsInactive();
}
