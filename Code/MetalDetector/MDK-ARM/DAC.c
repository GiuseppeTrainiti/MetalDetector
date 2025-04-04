/*
		The DAC is used to provide a DC voltage as bias to 
		the Q channel in order to zero the voltage at the 
		Q integrator when the voltage at the I integrator 
		has been already zeroed.
		
		Remember that the voltage in the I channel is 
		zeroed by moving the phase of the sample clocks.
		FIRST the I channel must be zeroed, THEN the Q channel.
		
		@TEST: PASSED
		
*/

#include "stm32f303x8.h"

void DACInit(){

	RCC->APB1ENR |= RCC_APB1ENR_DAC1EN;	// DAC clock enable
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	// Enable GPIOA
	
	GPIOA->MODER |= 1<<10;							// GPIOA5 in analog mode
	GPIOA->MODER |= 1<<11;
	
	DAC->CR |= 1<<3 | 1<<4 | 1<<5;			// DAC triggered by software
	//DAC->CR |= 1<<1;									// DAC output buffer enable CHECK THIS TO AVOID USING OPAMP AS BUFFER
	
	DAC->SWTRIGR |= 0;									// From bit 0 we trigger a conversion. It is done in the SysTick handler
	
	DAC->DHR12R1 = 0;									// DAC data register 
	
	DAC->CR |= 1<<0;										// DAC enable
	
}

