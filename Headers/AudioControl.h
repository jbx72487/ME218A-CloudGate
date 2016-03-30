
#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H

#include <stdint.h>

typedef enum {
	AUDIO_INTRO = 0, // hold looping trigger - loop until turn off
	AUDIO_LIQUID = 1, // basic trigger - play once and done
	AUDIO_SPRAYING = 2, // hold looping trigger - loop until turn off
	AUDIO_THUNDERSTORM = 3, // basic trigger - play once and done
	AUDIO_WINNING = 4, // hold looping trigger - loop until turn off
	AUDIO_LOSING = 5, // hold looping trigger - loop until turn off
	AUDIO_INSTRUCTION_MIX = 6, // basic trigger - play once and done
	AUDIO_INSTRUCTION_SPRAY = 7, // basic trigger - play once and done
	AUDIO_WRONG_BUTTON = 8

	} WhichSound_t; // up to 11 sounds

void AudioControlInit(void);
void setVolume(uint8_t vol);
void playSound(WhichSound_t sound);
void stopAllSounds(void);

#endif
