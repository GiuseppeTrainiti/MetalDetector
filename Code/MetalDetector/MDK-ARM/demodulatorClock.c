#include "stm32f303x8.h"
#include "frequencyMeasure.h"

/*
		This timer is used to generate the Quadrature and Phase signals that
		samples the RX signal. In order to be as precise as possible these 
		signals are generated using the RX measured period as a reference. 
		
		Once oscTime is measured, the compare register is loaded with oscTime/4,
		oscTime/2, 3/4(oscTime) and oscTime. In each of these time instants the
		appropriate pins are toggled in the timer ISR in order to generate the
		required signals.
		
		In order to account for ground balance there is also the possibility to
		add an offset to the phase of both Q and I signals. In this case the 
		compare register is loaded with K+oscTime/4, K+oscTime/2, K+3/4(oscTime) 
		and K+oscTime. Ideally K can be modified by the user.
		
		GPIOB1 and GPIOB5 (pin D6 and D11) are used for outputing respectively
		the Quadrature and Phase signals
		
		@ TEST: PASSED!

*/


extern int K;
extern int oscTime;
extern int nextQ, nextI, prevQ, prevI;

void timer15Init(){
	
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;			// GPIOB clock enable
	
	GPIOB->MODER |= 1<<2;										// GPIOB1 in output mode
	GPIOB->MODER &= ~(1<<3);
	
	GPIOB->MODER |= 1<<10;										// GPIOB5 in output mode
	GPIOB->MODER &= ~(1<<11);
	
	RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;		// Timer 15 clock enable
	
	TIM15->CR2 |= 0;							// Unused: various controls
	TIM15->SMCR |= 0;							// Unused: slave mode configuration
	TIM15->DIER |= 1<<1						// Interrupt enable on both compare match
							|  1<<2;
	TIM15->SR |= 0;								// Unused: interrupt flag register
	TIM15->EGR |= 0;							// Unused: event generation register
	TIM15->CCMR1 |= 0;						// Configured in output compare by default
	TIM15->CCER |= 0;							// Unused: waveform generation
	TIM15->CNT = 0;								// Reset counter value
	TIM15->PSC = 0;								// Timer runs at 64MHz
	TIM15->ARR = 0xFFFF;					// It will never reach it anyway
	TIM15->RCR = 0;								// Unused: repetition counter
	TIM15->CCR1 = K + oscTime/4;	// Quadrature compare
	TIM15->CCR2 = K;							// Phase compare
	TIM15->BDTR |= 0;							// Unused: break and dead time insertion
	TIM15->DCR |= 0;							// Unused: DMA configuration
	TIM15->DMAR |= 0;							// Unused: DMA address target
	
	NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
	//NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, 1);
}

void TIM1_BRK_TIM15_IRQHandler(){	
	if(TIM15->SR & 1<<1){
		/* **** QUADRATURE COMPARE MATCH **** */
		TIM15->SR &= ~(1<<1);
		
		if(TIM15->CCR1 == prevQ){
			GPIOB->BSRR = 1<<1;										// Q signal is high
			TIM15->CCR1 = nextQ;					// Update the compare value
		}
		else if(TIM15->CCR1 == nextQ){
				GPIOB->BSRR = 1<<17;								// Q signal is low
				TIM15->CR1 &= ~(1<<0);							// TIM15 is stopped. It will be started again by EXTI0 TIM17.
			  TIM15->CNT = 0;
		}
		/* ********************************** */
	}
	
	else{
		/* **** PHASE COMPARE MATCH **** */
		TIM15->SR &= ~(1<<2);
		if(TIM15->CCR2 == prevI){
			GPIOB->BSRR = 1<<21;									// I signal is low
			TIM15->CCR2 = nextI;						// Update the compare value
		}
		else if(TIM15->CCR2 == nextI){
			GPIOB->BSRR = 1<<5;										// I signal is high. Next compare value will be set by EXTI0 TIM17
		}
		/* ***************************** */		
	}
}

