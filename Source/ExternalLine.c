// #define TEST

/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/

#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
 
 #include "termio.h"
#include "BITDEFS.H"


/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff<<2)
#define EXTERNAL_LINE_PIN BIT1HI

/*---------------------------- Module Functions ---------------------------*/


/*---------------------------- Module Variables ---------------------------*/


/*------------------------------ Module Code ------------------------------*/


void ExternalLineInit (void) {
		// Initialize Tiva to interact with shift register: Port F Pins 1

	//enable the clock to Port F by setting bit 5 in register
	HWREG(SYSCTL_RCGCGPIO) |= BIT5HI;

	// wait for clock to be ready
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R5) != BIT5HI)
	;

	// Connect to digital I/O port by writing to Digital Enable register
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= EXTERNAL_LINE_PIN;

	// Set data direction  to output for these pins by writing to direction register
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= EXTERNAL_LINE_PIN;

	// set initial value to low
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~EXTERNAL_LINE_PIN;

}

void setExternalLineHi (void) {
	// set output to high
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) |= (EXTERNAL_LINE_PIN);

}

void setExternalLineLo (void) {
	// set output to lo
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~(EXTERNAL_LINE_PIN);

}
