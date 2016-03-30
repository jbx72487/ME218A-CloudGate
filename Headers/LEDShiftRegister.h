/****************************************************************************
 
  Header file for the LEDShiftRegister Module

 ****************************************************************************/
 
#ifndef LED_SHIFT_REGISTER_H
#define LED_SHIFT_REGISTER_H

void LED_SR_Init(void);
uint32_t LED_SR_GetCurrentRegister(void);
void LED_SR_Write(uint32_t NewValue);

#endif // LED_SHIFT_REGISTER_H
