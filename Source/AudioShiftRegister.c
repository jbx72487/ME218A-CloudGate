// #define TEST

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"

#include "BITDEFS.H"
#include "ES_Port.h"

#include "EnablePA25_PB23_PD7_PF0.h"


#include "AudioShiftRegister.h"

/******************** Module Defines ***********************/

#define AUDIO_DATA BIT4HI
#define AUDIO_RCLK BIT3HI
#define AUDIO_SCLK BIT2HI
#define AUDIO_SR_TIVA_PINMASK (AUDIO_DATA | AUDIO_SCLK| AUDIO_RCLK)
#define SIZE_OF_SR 16
#define MSB_MASK (BIT0HI << (SIZE_OF_SR-1))
#define ALL_BITS (0xff<<2)

/******************** Private Functions ***********************/

/******************** Module Variables ***********************/

// declare currentRegisterVal as private module variable
static uint16_t currentRegisterVal = 0;

/******************** Module Code ***********************/

void Audio_SR_Init(void) {

	// Initialize Tiva to interact with shift register: Port A Pins 2, 3, 4

	//enable the clock to Port A by setting bit 0 in register
	HWREG(SYSCTL_RCGCGPIO) |= BIT5HI;

	// wait for clock to be ready
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R5) != BIT5HI)
	;

	// Connect Port B Pin 0,1,2 to digital I/O port by writing to Digital Enable register
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= AUDIO_SR_TIVA_PINMASK;

	// Set data direction  to output for these pins by writing to direction register
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= AUDIO_SR_TIVA_PINMASK;

	// set initial values for data, shift clock, and register clock to low
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~AUDIO_SR_TIVA_PINMASK;

}

uint16_t Audio_SR_GetCurrentRegister(void) {
	// Return currentRegisterVal
	// puts("Getting current register.\n\r");
	return currentRegisterVal;
}

void Audio_SR_Write(uint16_t NewValue) {
	// Loop through the bits one at a time
	uint16_t tempValue = NewValue;
	// printf("Entered Audio_SR_Write to write %#04x\n\r",NewValue);
	// printf("This should be the same as %#04x\n\r",tempValue);
	for (uint16_t i = 0; i < SIZE_OF_SR; i++) {
		// printf("Loop: %d\n\r",i);
		// load MSB of NewValue into the shift register (set PB0 to the value of that bit)
		// printf("Checking 7th bit of value: %d\n\r",tempValue & BIT7HI);
		if ((tempValue & MSB_MASK) == MSB_MASK) {
		// if MSB is high, make PB0 high
			HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) |= (AUDIO_DATA);
			// puts("MSB is high -> made PB0 high\n\r");
		// otherwise make it low
		} else {
			// puts("MSB is low -> about to make PB0 low\n\r");
			HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~(AUDIO_DATA);
			//puts("MSB is low -> made PB0 low\n\r");
		}
		  //puts("About to pulse shift clock\n\r");

		// pulse the shift clock (set PB1 high, then low)
		HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) |= (AUDIO_SCLK);
		HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~(AUDIO_SCLK);
		//puts("Done pulsing shift clock. About to shift to look at next MSB\n\r");

		// update tempValue
		tempValue = tempValue << 1;
	}
	// puts("Exited Audio_SR_Write for-loop\n\r");

	// pulse the register clock (set PB2 high, then low)	
	// set output to high
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) |= (AUDIO_RCLK);
	// set output to low
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~(AUDIO_RCLK);

	// update currentRegisterVal
	currentRegisterVal = NewValue;
	// puts("Exited Audio_SR_Write\n\r");

}

// test harness

#ifdef TEST
#include "termio.h"

int main(void)
{
	TERMIO_Init();
	PortFunctionInit();
	puts("\r\n In test harness for Part 2.7: ShiftRegisterControl (1)\r\n");
	Audio_SR_Init();
	uint16_t test_val = 0xc5;
	Audio_SR_Write(test_val);
	puts("\r\n Writing 0xc5: 1100 0101\r\n");
	// puts("Current register value:");
  // putchar(Audio_SR_GetCurrentRegister());
	return 0;
}
#endif
