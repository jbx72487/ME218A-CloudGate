
#ifndef AUDIO_SHIFT_REGISTER_H
#define AUDIO_SHIFT_REGISTER_H

#include <stdint.h>

void Audio_SR_Init(void);
uint16_t Audio_SR_GetCurrentRegister(void);
void Audio_SR_Write(uint16_t NewValue);

#endif
