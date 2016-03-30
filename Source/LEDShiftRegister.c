//#define SHIFT_TEST
/****************************************************************************
 Module
   LEDShiftRegister.c

 Revision
   1.0.1

 Description
   This module acts as the low level interface to a write only shift register.

 Notes

 History
 When           	Who     What/Why
 -------------- 	---     --------
 11/07/15	22:52	jam		converted to 32-bit
 10/17/15	18:35 	jam		Updated with implementation
 10/11/15 	19:55 	jec     first pass
 
****************************************************************************/
// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "BITDEFS.H"

// readability defines
#define DATA GPIO_PIN_0

#define SCLK GPIO_PIN_3
#define SCLK_HI BIT3HI
#define SCLK_LO BIT3LO

#define RCLK GPIO_PIN_2
#define RCLK_LO BIT2LO
#define RCLK_HI BIT2HI

#define GET_MSB_IN_LSB(x) ((x & 0x80000000)>>31)
#define ALL_BITS (0xff<<2)

// an image of the last 8 bits written to the shift register
static uint32_t LocalRegisterImage=0;

/****************************************************************************
 Function
	LED_SR_Init

 Parameters
	None

 Returns
	None
 
 Description
	Initializes the 3 output ports on the Tiva that will be used as inputs into
	the shift registers.
   
 Author
	Jordan Miller, 10/17/15, 06:35 PM
****************************************************************************/
void LED_SR_Init(void){

  // set up port B by enabling the peripheral clock and setting the direction
  // of PB2, PB2 & PB3 to output
  if((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1) {
		HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
		while((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
	}
	
	HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (DATA | SCLK | RCLK);
	HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (DATA | SCLK | RCLK);
	
  // start with the data & sclk lines low and the RCLK line high
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS))  &= ~(DATA | SCLK);
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS))  |= RCLK;
}

/****************************************************************************
 Function
	LED_SR_GetCurrentRegister

 Parameters
	None

 Returns
	uint32_t that represents the state of the registers currently

 Description
	Gets the current state of the output pins of the register
   
 Author
	Jordan Miller, 10/17/15, 06:35 PM
****************************************************************************/
uint32_t LED_SR_GetCurrentRegister(void){
  return LocalRegisterImage;
}

/****************************************************************************
 Function
	LED_SR_Write

 Parameters
	uint32_t NewValue that represents the desired output values of the pins

 Returns
	none

 Description
	Writes the desired NewValue to the output pins of the shift registers
   
 Author
	Jordan Miller, 10/17/15, 06:35 PM
****************************************************************************/
void LED_SR_Write(uint32_t NewValue) {

  LocalRegisterImage = NewValue; // save a local copy

	// lower the register clock
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS))  &= RCLK_LO;
	
	// shift out the data while pulsing the serial clock
	uint8_t DataPin;
	for(uint8_t i=0;i<32;i++) {
		// Isolate the MSB of NewValue, put it into the LSB position and output
		DataPin = GET_MSB_IN_LSB(NewValue);
		// printf("Data pin: %#10x\r\n",DataPin);

		if (DataPin == 0) {
			// printf("0");
			HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= ~DATA;
		} else {
			// printf("1");
			HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= DATA;
		}
		// raise SCLK
		HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= SCLK_HI;	
		// lower SCLK
		HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= SCLK_LO;
		
		NewValue = NewValue << 1; // Move to the next bit
			
	}
	// printf("PortB value: %#10x\r\n",HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)));

	// raise the register clock to latch the new data
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS))  |= RCLK_HI;
  
}

#ifdef SHIFT_TEST
 #include "termio.h"
#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

#include "EnablePA25_PB23_PD7_PF0.h"

int main(void) {
		TERMIO_Init();
	clrScrn();
	
	// When doing testing, it is useful to announce just which program
	// is running.
	puts("\rStarting Test Harness for \r");
	printf("the 2nd Generation Events & Services Framework V2.2\r\n");
	printf("%s %s\n",__TIME__, __DATE__);
	printf("\n\r\n");
	printf("Press any key to post key-stroke events to Service 0\n\r");
	printf("Press 'd' to test event deferral \n\r");
	printf("Press 'r' to test event recall \n\n\r");
	PortFunctionInit();
	LED_SR_Init();
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS))  &= ~DATA;
	
	LED_SR_Write(0xaaaaaaaa);
	
	return 0;
}

#endif
