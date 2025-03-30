#include "stm32f303x8.h"

/*
	This code is used to manage the encoder knob. It is usage-independent,
	meaning that it only sense if it is turned and update a counter variable
	accordingly, decreasing or increasing it based on the turn direction.
	
	It also manages the pushbutton embedded into the encoder.
	
			D8 (PF1): Phase A used as interrupt EXTI1 reference
			D3 (PB0): Phase B
			A7 (PA2): Pushbutton EXTI2
			
			@TEST: PASSED
	
*/

#define KSTEP 1
#define QSTEP 1

extern int encoderCounter;
extern int K;

void EXTI1Config(){
	RCC->AHBENR |= RCC_AHBENR_GPIOFEN;		// Enable GPIOF clock 
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;		// Enable GPIOB clock
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;	// Enable SysConfig clock
	
	GPIOF->MODER &= ~(1<<2);							// GPIOF1 in input mode
	GPIOF->MODER &= ~(1<<3);
	
	GPIOF->PUPDR |= 1<<2;									// GPIOF1 internal pullup enable
	GPIOF->PUPDR &= ~(1<<3);
	
	SYSCFG->EXTICR[0] |= 5<<4;						// EXTI1 assigned to GPIOF0
	EXTI->IMR |= 1<<1;										// EXTI1 unmasked
	EXTI->FTSR |= 1<<1;										// EXTI1 sensitive to falling edge
	
	GPIOB->MODER &= ~(1<<0);							// GPIOB0 in input mode
	GPIOB->MODER &= ~(1<<1);

	GPIOB->PUPDR |= 1<<0;									// GPIOB0 internal pullup enable
	GPIOB->PUPDR &= ~(1<<1);
	
	NVIC_EnableIRQ(EXTI1_IRQn);
	//NVIC_SetPriority(EXTI1_IRQn, 2);
}

void EXTI2Config(){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		// Enable GPIOA clock 
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;	// Enable SysConfig clock
	
	GPIOA->MODER &= ~(1<<4);							// GPIOA2 in input mode
	GPIOA->MODER &= ~(1<<5);
	
	GPIOA->PUPDR |= 1<<4;									// GPIOA2 internal pullup enable
	GPIOA->PUPDR &= ~(1<<5);
	
	SYSCFG->EXTICR[0] |= 0;								// EXTI2 assigned to GPIOA2 by default
	EXTI->IMR |= 1<<2;										// EXTI2 unmasked
	EXTI->FTSR |= 1<<2;										// EXTI2 sensitive to falling edge
	
	NVIC_EnableIRQ(EXTI2_TSC_IRQn);
	//NVIC_SetPriority(EXTI2_TSC_IRQn, 3);
}

void EXTI1_IRQHandler(){									// ENCODER ROTATION
	EXTI->PR |= 1<<1;												// Clear pending EXTI1 interrupt flag
		
	if(encoderCounter == 0){								// CASE 0: THE ENCODER CHANGES THE DEMOD CLOCK PHASES
		if(GPIOB->IDR & (1<<0)) K = K+KSTEP;			
		else K = K-KSTEP;										
	}
	
	else if(encoderCounter == 1){																	// CASE 1: THE ENCODER CHANGES THE Q SIGNAL OFFSET
		if(GPIOB->IDR & (1<<0)) DAC->DHR12R1 = DAC->DHR12R1 + QSTEP;			
		else DAC->DHR12R1 = DAC->DHR12R1 - QSTEP;
		//DAC->SWTRIGR |= 1<<0;																				// DAC output trigger
	}
}

void EXTI2_TSC_IRQHandler(){							// ENCODER PUSH
		EXTI->PR |= 1<<2;											// Clear pending EXTI2 interrupt flag
		encoderCounter = 1-encoderCounter;		// Allows to switch form parameter to parameter
}