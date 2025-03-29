/*

@Author: Giuseppe Trainiti
@Date:   15/02/2025
@Brief:  this code generates two identical signal with a dead time
				 insertion in the switching phase in order to drive a pair
				 of BJT used to switch an inverting charge pump
				 
@Timer:  The time base of the timer includes several registers:

				 - Counter register CNT
					 Counter value 16 bit register. It starts counting one clock cycle
					 after the CEN bit is set
				 - Prescaler register PSC
					 Divides the clock frequency in a range between 1 and 65536 and it
					 can be modified on the fly but it is considered valid only after
					 an update event
				 - Auto reload register ARR:
					 We write and read on a preload register that is actually loaded
					 into the auto reload register only when an update event occours
				 - Repetition counter register RCR:
					 An update event is triggered only when the timer generates a number
					 of updates equal to the RCR value. Each time an overflow event happends
					 it is decremented, and when it reaches 0, an update event is triggered.
	
				 The interrupt bit flags are stored into the SR register, specifically bit 1 is
				 set when there is a compare match, while bit 0 is set when an update event occours.
	
@Hardware: pin 13 (PB3) is used to drive the NPN transistor, while pin 12 (PB4) is used
					 to drive the PNP transistor.
					 
					 @Test:	PASSED	
*/

#include "stm32f303x8.h"
#include "invertingChargePump.h"

#define START 5								// Init value for output compare register

void timer16Init(){
	RCC->APB2ENR |= 1<<17;    	// Timer 16 clock enable
	
	
	RCC->AHBENR |= 1<<18;				// GPIOB clock enabled
	
	GPIOB->MODER |= 1<<6;				// Pin13 (PB3) is an output
	GPIOB->MODER &= ~(1<<7); 
	
	GPIOB->MODER |= 1<<8;				// Pin12 (PB4) is an output
	GPIOB->MODER &= ~(1<<9); 
	
	GPIOB->BSRR = 1<<19;				// NPN signal starts at 0
	GPIOB->BSRR = 1<<4;					// PNP signal starts at 1
	
	
	TIM16->CR2 |= 0;						// Unused: configures waveform generation
	TIM16->DIER |= 1<<1 | 1<<0;	// Interrupt enabled on update event and compare match event
	TIM16->SR |= 0;							// Interrupt flag register
	TIM16->EGR |= 0;						// Unused: event generation register
	TIM16->CCMR1 |= 0;					// Capture/Compare channel 1 is by default configured as an output compare channel
	TIM16->CCER |= 1<<0;				// Output compare module enable	
	TIM16->CNT = 0;							// Reset counter value
	TIM16->PSC = 9;							// Timer is fed with 6.4MHz for Ttick = 156.25ns
	TIM16->ARR = 960;						// Timer overflows in 150us for Fovfl = 6.67kHz
	TIM16->RCR = 0;							// No need to use the repetition counter
	TIM16->CCR1 = START;				// Assignment of the output compare value
	TIM16->BDTR |= 0;						// Unused: break and dead time generation register
	TIM16->DCR |= 0;						// Unused: DMA control register
	TIM16->DMAR |= 0;						// Unused: DMA transfer address
	TIM16->OR |= 0;							// Unused: select the input TI1
	
	NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
	//NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 4);
	
	TIM16->CR1 |= 1<<0;					// Timer enabled

	
}

void TIM1_UP_TIM16_IRQHandler(void){
	if(TIM16->SR & 1<<0){
		/* ********** UPDATE EVENT ********** */
		TIM16->SR &= ~(1<<0);
		/* ********************************** */
	}
	if(TIM16->SR & 1<<1){
		/* ******* COMPARE MATCH EVENT ******* */
		TIM16->SR &= ~(1<<1);
		switch(TIM16->CCR1){
			case 5:
				GPIOB->ODR &= ~(1<<4);
				TIM16->CCR1 = 475;
				break;
			case 475:
				GPIOB->ODR |= 1<<4;
				TIM16->CCR1 = 485;
				break;
			case 485:
				GPIOB->ODR |= 1<<3;
				TIM16->CCR1 = 955;
				break;
			case 955:
				GPIOB->ODR &= ~(1<<3);
				TIM16->CCR1 = 5;
				break;
			default:
			break;
		}
		/* *********************************** */
	}
}