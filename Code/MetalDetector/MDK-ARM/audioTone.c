#include "stm32f303x8.h"
/*
		This code generate a sound that is derived by a Carthesian 
		interpretation of the I and Q signals. 
		
		The Q signals generate a low pitch tone, while the I signal
		generates an High pitch noise. 
		
		Both increase in frequency proportionately to the single
		signal strenght. The discrimination is performed by hearing
		the different ratios by the high and low pitch sounds.
		
		PRO: very easy to balance and tare the detector
		CONS: I am not able to distinguis various tone ratios
		
		TODO: write the code to generate a sound derived by a POLAR
		interpretation of the QI signals, where the thresold is the 
		module of the vector and the pitch is determined by the phase
		angle. 
		
		@TEST: PASSED
*/

extern int ARRbaseLow, ARRbaseHigh;
extern int Qthreshold, Ithreshold;
extern int Itone, Qtone;
extern int I, Q;


void toneInit(){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	
	GPIOA->MODER |= 1<<10;
	GPIOA->MODER &= ~(1<<11);
}

void TIM6Init(){	
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;			// Timer clock enable
	TIM6->CR2 |= 0;													// Unused
	TIM6->DIER |= 1<<0;											// Enable interrupt on update
	TIM6->SR |= 0;													// Interrupt flag register
	TIM6->CNT |= 0;													// Counter variable
	TIM6->PSC = 10;													// Prescaler
	TIM6->ARR = ARRbaseLow;
	TIM6->CR1 |= 1<<0;											// Timer enable
	NVIC_SetPriority(TIM6_DAC1_IRQn, 2);		// NVIC priority
	NVIC_EnableIRQ(TIM6_DAC1_IRQn);					// NVIC enable
}

void TIM7Init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;			// Timer clock enable
	TIM7->CR2 |= 0;													// Unused
	TIM7->DIER |= 1<<0;											// Enable interrupt on update
	TIM7->SR |= 0;													// Interrupt flag register
	TIM7->CNT |= 0;													// Counter variable
	TIM7->PSC = 10;													// Prescaler
	TIM7->ARR = ARRbaseHigh;
	TIM7->CR1 |= 1<<0;											// Timer enable
	NVIC_SetPriority(TIM7_DAC2_IRQn, 2);		// NVIC priority
	NVIC_EnableIRQ(TIM7_DAC2_IRQn);					// NVIC enable
}

/* CARTHESIAN TONES */
void TIM6_DAC1_IRQHandler(){
	TIM6->SR &= ~(1<<0);
	
	if(Q <= - Qthreshold || Q >= Qthreshold){
		TIM6->ARR = ARRbaseLow + Q*2;
		Qtone = 1-Qtone;
	}
	else Qtone = 0;
	
	if(Qtone || Itone) GPIOA->ODR &= ~(1<<5);
	else GPIOA->ODR |= 1<<5;
}

void TIM7_DAC2_IRQHandler(){
	TIM7->SR &= ~(1<<0);
	
	if(I <= - Ithreshold || I >= Ithreshold){
		Itone = 1-Itone;
		TIM7->ARR = ARRbaseHigh + I*2;   // offset performed at acquisition
	}
	else Itone = 0;
	
	if(Qtone || Itone) GPIOA->ODR &= ~(1<<5);
	else GPIOA->ODR |= 1<<5;
}
