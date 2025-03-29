#include "stm32f303x8.h"

/*
		This file implement the tone generation for target discrimination.
		It uses the argument variable in order to generate the tone. All significant
		target causes signal in the lower left quadrant, meaning that the
		arctg can go from 0 to -90.
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
	TIM6->PSC = 20;													// Prescaler
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
	TIM7->PSC = 7;													// Prescaler
	TIM7->ARR = ARRbaseHigh;
	TIM7->CR1 |= 1<<0;											// Timer enable
	NVIC_SetPriority(TIM7_DAC2_IRQn, 2);		// NVIC priority
	NVIC_EnableIRQ(TIM7_DAC2_IRQn);					// NVIC enable
}

void TIM6_DAC1_IRQHandler(){
	TIM6->SR &= ~(1<<0);
	
	if(Q <= -Qthreshold/*(2048-Qthreshold)*/ || Q >= Qthreshold/*(2048+Qthreshold)*/){
		TIM6->ARR = ARRbaseLow + Q /*(Q - 2048)*/;
		Qtone = 1-Qtone;
	}
	else Qtone = 0;
	
	if(Qtone || Itone) GPIOA->ODR &= ~(1<<5);
	else GPIOA->ODR |= 1<<5;
}

void TIM7_DAC2_IRQHandler(){
	TIM7->SR &= ~(1<<0);
	
	if(I <= -Ithreshold/*(2048-Ithreshold)*/ || I >= Ithreshold/*(2048+Ithreshold)*/){
		Itone = 1-Itone;
		TIM7->ARR = ARRbaseHigh + /*(I - 2048)*/ I;   // offset performed at acquisition
	}
	else Itone = 0;
	
	if(Qtone || Itone) GPIOA->ODR &= ~(1<<5);
	else GPIOA->ODR |= 1<<5;
}
