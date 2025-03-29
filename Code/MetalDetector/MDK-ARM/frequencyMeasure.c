#include "stm32f303x8.h"
#include "frequencyMeasure.h"
/*
	This code is used to measure the frequency of the transmitted signal.
	The signal is fed to the microcontroller via GPIOF0 (D7) through a comparator.
	The pin is configured as an external interrupt, triggered on the
	falling edge. 
	
	When the interrupt is triggered, TIM17 starts counting and when it is
	triggered again, it stops. Now the counter value is equal to the period
	of the transmitted signal. 
	
	@Test: PASSED
	
*/

extern int K;
extern int acquisition;
extern int oscTime;
extern int nextI, nextQ, prevI, prevQ;

void EXTI0Config(){
	RCC->AHBENR |= RCC_AHBENR_GPIOFEN;		// Enable GPIOF clock 
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;	// Enable SysConfig clock
	
	GPIOF->MODER &= ~(1<<0);							// GPIOF0 in input mode
	GPIOF->MODER &= ~(1<<1);
	
	GPIOF->PUPDR |= 1<<0;									// GPIOF0 internal pullup enable
	GPIOF->PUPDR &= ~(1<<1);
	
	SYSCFG->EXTICR[0] |= 1<<0 | 1<<2;			// EXTI0 assigned to GPIOF0
	EXTI->IMR |= 1<<0;										// EXTI0 unmasked
	EXTI->FTSR |= 1<<0;										// EXTI0 sensitive to falling edge
	
	NVIC_EnableIRQ(EXTI0_IRQn);
	//NVIC_SetPriority(EXTI0_IRQn, 0);
}

void timer17Init(){
	RCC->APB2ENR |= 1<<18;    							// Timer 17 clock enable	
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;			// GPIOB clock enable

	
	TIM17->CR2 |= 0;						// Unused: configures waveform generation
	TIM17->DIER |= 0;						// Unused: no interrupts required
	TIM17->SR |= 0;							// Unused: interrupt flag register
	TIM17->EGR |= 0;						// Unused: event generation register
	TIM17->CCMR1 |= 0;					// Unused: capture/compare channel 1 is by default configured as an output compare channel
	TIM17->CCER |= 0;						// Unused: output compare enable
	TIM17->CNT = 0;							// Reset counter value
	TIM17->PSC = 0;							// Timer is fed with 64MHz for Ttick = 15.625ns
	TIM17->ARR = 0xFFFF;				// Timer overflows in 150us for Fovfl = 6.67kHz
	TIM17->RCR = 0;							// No need to use the repetition counter
	TIM17->CCR1 = 0;						// Unused: capture compare value
	TIM17->BDTR |= 0;						// Unused: break and dead time generation register
	TIM17->DCR |= 0;						// Unused: DMA control register
	TIM17->DMAR |= 0;						// Unused: DMA transfer address
	TIM17->OR |= 0;							// Unused: select the input TI1
}

void EXTI0_IRQHandler(){	
	EXTI->PR |= 1<<0;						// Clear pending EXTI0 interrupt flag

	if(acquisition == 0){				// First edge - Timer is started
		TIM17->CR1 |= 1<<0;
		acquisition = 1;
	}
	else if(acquisition == 1){	// Second edge - Result is ready
		oscTime = TIM17->CNT;
		TIM17->CR1 &= ~(1<<0);
		TIM17->CNT = 0;
		acquisition = 0;
	}	
	
	TIM15->CCR2 = K;						// First threshold for I
	TIM15->CR1 |= 1<<0;					// Timer 15 started	
	TIM15->CCR1 = K+oscTime/4;	// First threshold for Q
	
	prevI = K;									// First threshold stored in some variables to
	prevQ = K+oscTime/4;				// allow a faster comparison in TIM15 IRQ
	nextQ = K+3*oscTime/4;			// Already evaluating the next threshold for Q
	nextI = K + oscTime/2;			// Already evaluating the next threshold for I
}
