#include "stm32f303xc.h"

/*
		This code reads the demodulated signals and assigns
		them to the relative variable after averaging them.
		
		The conversion is triggered by Systick
		
		@TEST: PASSED
*/

#define ADCOFFSET 2048  // This is the ADC conversion of the ADC BIAS
#define AVG	256

extern int Q, I;

volatile int Qacc = 0, Iacc = 0, i = 0, j = 0;

void ADC1_Init(void) {
    // Enable clock for GPIOA and ADC1
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_ADC12EN;

    // Configure GPIOA pins 0 (PA0) and 1 (PA1) as analog mode
    GPIOA->MODER |= (3U << (0 * 2)) | (3U << (1 * 2));

    // Set ADC1 clock prescaler and clock source
    RCC->CFGR2 |= RCC_CFGR2_ADCPRE12_DIV4; // Set ADC clock prescaler
    ADC12_COMMON->CCR &= ~ADC12_CCR_CKMODE; // Select synchronous clock mode (HCLK/1)

    // Enable ADC1 voltage regulator
    ADC1->CR &= ~ADC_CR_ADVREGEN;
    ADC1->CR |= ADC_CR_ADVREGEN_0;

    // Configure ADC resolution and alignment
    ADC1->CFGR &= ~ADC_CFGR_RES; // 12-bit resolution
    ADC1->CFGR &= ~ADC_CFGR_ALIGN; // Right alignment
    ADC1->CFGR &= ~ADC_CFGR_CONT;  // Ensure single conversion mode

    // Enable ADC1
    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)); // Wait for ADC to be ready
		
		ADC1->IER |= 1<<2;										// Interrupt enable on EOC
		NVIC_SetPriority(ADC1_2_IRQn, 2);
		NVIC_EnableIRQ(ADC1_2_IRQn);
}

void ADC1_Read(uint8_t channel) {
  ADC1->SQR1 = 0; 														// Clear previous selection  
	
	if (channel == 1) ADC1->SQR1 |= 1<<6; 			// Select channel 1 (PA0)  
	else if (channel == 2) ADC1->SQR1 |= 1<<7; 	// Select channel 2 (PA1)

	ADC1->CR |= ADC_CR_ADSTART;									// Start conversion
}

void ADC1_2_IRQHandler(){
	ADC1->ISR = 1<<2;				// Clear EOC interrupt flag
	
	if(ADC1->SQR1 & 1<<6){											// Sampling Q
		if(i < AVG){
			Qacc = ADC1->DR - ADCOFFSET + Qacc;
			i++;
		}
		else{
			Q = Qacc/AVG;
			Qacc = 0;
			i = 1;
		}
	}
	
		if(ADC1->SQR1 & 1<<7){											// Sampling I
		if(j < AVG){
			Iacc = ADC1->DR - ADCOFFSET + Iacc;
			j++;
		}
		else{
			I = Iacc/AVG;
			Iacc = 0;
			j = 1;
		}
	}
	
}
