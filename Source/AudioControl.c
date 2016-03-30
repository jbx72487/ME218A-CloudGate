// #define TEST

#include <stdint.h>
#include <stdbool.h>


#include "AudioShiftRegister.h"
#include "AudioControl.h"
#include "BITDEFS.H"

/******************** Module Defines ***********************/
#define ALL_BITS (0xff<<2)

#define TARGET_VOLUME 10

#define VOL_UP BIT0LO
#define VOL_DOWN BIT1LO
#define SOUND_0 BIT2LO
#define NUM_AUDIO_PINS 11
#define ALL_AUDIO_PINS_HI (BIT2LO | BIT3LO | BIT4LO | BIT5LO | BIT6LO | BIT7LO | BIT9LO | BIT10LO | BIT11LO | BIT12LO | BIT13LO) // skipped BIT 8 for ease of setup of connector pins

/******************** Private Functions ***********************/

void volumeUp(void);
void volumeDown(void);

/******************** Module Variables ***********************/

// declare currentRegisterVal as private module variable
static uint8_t currentVolume;
static uint16_t soundArray[NUM_AUDIO_PINS] = {BIT2LO, BIT3LO, BIT4LO, BIT5LO, BIT6LO, BIT7LO, BIT9LO, BIT10LO, BIT13LO, BIT11LO, BIT12LO}; // skipped BIT 8 for ease of setup of connector pins

/******************** Module Code ***********************/

void AudioControlInit(void) {
	Audio_SR_Init();
	currentVolume = 0;
	// set all values to high
	stopAllSounds();
}

void setVolume(uint8_t vol) {
	if (TARGET_VOLUME > 0) {
		for (int i = 0; i < TARGET_VOLUME; i++) {
			volumeUp();
		}
	} else {
		for (int i = 0; i < TARGET_VOLUME; i++) {
			volumeDown();
		}
	}
}

void playSound(WhichSound_t sound) {
	// sets corresponding pin low on SR & makes the rest high (so that we only play one song at once)
	Audio_SR_Write(soundArray[sound]);
}

void stopAllSounds(void) {
	// sets all pins high on SR
	Audio_SR_Write(ALL_AUDIO_PINS_HI);
}

void volumeUp(void) {
	// pulse volume up pin LO, then HI
	uint8_t currentRegister = Audio_SR_GetCurrentRegister();
	Audio_SR_Write(currentRegister & ~VOL_UP);
	Audio_SR_Write(currentRegister | VOL_UP);

	currentVolume++;
}

void volumeDown(void) {
	// set volume down pin LO, then HI
	uint8_t currentRegister = Audio_SR_GetCurrentRegister();
	Audio_SR_Write(currentRegister & ~VOL_DOWN);
	Audio_SR_Write(currentRegister | VOL_DOWN);
	currentVolume--;
}

// test harness

#ifdef TEST
#include "termio.h"

int main(void)
{
	TERMIO_Init();
	PortFunctionInit();
	puts("\r\n In test harness for Part 2.7: ShiftRegisterControl (1)\r\n");
	AudioControlInit();
	char input = getchar();
	while (input != 'a') {
		if (input >= '0' && input <=('0'+NUM_AUDIO_PINS-1)) {
			playSound(input-'0');
			printf("Current register: %x",Audio_SR_GetCurrentRegister());
		}
		input = getchar();
	}
	return 0;
}
#endif
