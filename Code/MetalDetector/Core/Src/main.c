#include "main.h"

#include "math.h"
#include "arm_math.h"

#include "invertingChargePump.h"
#include "frequencyMeasure.h"
#include "demodulatorClock.h"
#include "rotaryEncoder.h"
#include "ADC.h"
#include "DAC.h"
#include "audioTone.h"

#define SYSTICK	20					// Time in ms for SysTick interrupts


I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);

volatile int acquisition = 0;																// Used to measure the TX frequency
volatile int oscTime = 6000;																// TX frequency value
volatile int K = 1000;																			// Phase shift of the demod clocks
volatile int nextI, nextQ, prevI, prevQ;										// Used as threshold to generate the demod clocks
volatile int encoderCounter = 0;														// Used to handle the rotation of the encoder
volatile int Q, I;																					// Values of the demodulated RX signal

volatile int evaluateAngle = 0;															// Used to trigger arctg calculations

/* TONE AUDIO VERSION A - CARTHESIAN APPROACH */
volatile int ARRbaseLow = 9000, ARRbaseHigh = 5000;			
volatile int Qthreshold = 40, Ithreshold = 40, magnitudeThreshold = 280;
volatile int Itone = 0, Qtone = 0;

/* RX VECTOR DATA */
float magnitude, phase;


int main(void){
	
	/*
			-- INVERTING CHARGE PUMP PINS --
			D13 (PB3): NPN signal for inverting charge pump
			D12 (PB4): PNP signal for inverting charge pump
			
			-- FREQUENCY MEASURE PIN --
			D7 (PF0): TX sync input from comparator
			
			-- DEMODULATOR CLOCK PINS --
	
			D6 (PB1): Quadrature demodulator clock
			D11 (PB5): Phase demodulator clock
	
			-- ENCODER PINS --
			
			D8 (PF1): Phase A used as interrupt reference
			D3 (PB0): Phase B
			A7 (PA2): Pushbutton
	
			-- I2C PINS -- 
			
			D4 (PB7): SDA
			D5 (PB6): SCL
			
			-- ADC PINS --
			
			A0 (PA0): IN1 - Q
			A1 (PA1): IN2 - I
			A2 (PA3): IN4 - ADC BIAS
			
			-- TONE PIN --
			
			A4 (PA5): Audio
			
			-- DAC PIN --
			
			A3 (PA4): Q bias
			
			------------------------------
	*/
	
	/* ********** SYSTEM INITIALIZATION BY IDE ********** */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
	/* ************************************************** */

	/* ********** INVERTING CHARGE PUMP SIGNAL GENERATION ********* */
	timer16Init();
	/* ********** QUADRATURE AND PHASE SIGNAL GENERATION ********** */
	timer15Init();
	/* ************* FREQUENCY MEASURE INITIALIZATION ************* */
  timer17Init();
	EXTI0Config();
	/* ************* ROTARY ENCODER INITIALIZATION ************* */
	EXTI1Config();
	EXTI2Config();
	/* ******************* SysTick CONFIGURATION ****************** */
	SysTick->CTRL |= 1<<1 | 1<<0;		
	SysTick->LOAD = 6400; 					// 100 us							  	
	/* ************* ANALOG TO DIGITAL CONVERSION ************ */
	ADC1_Init();
	/* ************* DIGITAL TO ANALOG CONVERSION ************ */
	DACInit();
	/* ************* TONE GENERATION INITIALIZATION ************* */
	TIM6Init();
	TIM7Init();
	toneInit();
	/* ************************************************************ */
	
  while (1){
		if(evaluateAngle){
			evaluateAngle = 0;
			magnitude = sqrt(pow(Q, 2)+pow(I, 2));
			phase = atan((float)Q/I)*100;
		}
  }
}

void SystemClock_Config(void){
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){Error_Handler();}

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK){Error_Handler();}
	
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){Error_Handler();}
	
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){Error_Handler();}
}

static void MX_I2C1_Init(void){
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00201D2B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  
	if (HAL_I2C_Init(&hi2c1) != HAL_OK){Error_Handler();}
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK){Error_Handler();}
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK){Error_Handler();}
}

static void MX_SPI1_Init(void){
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK){Error_Handler();}
}

static void MX_GPIO_Init(void){
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

void Error_Handler(void){
  __disable_irq();
  while(1) {}
}
