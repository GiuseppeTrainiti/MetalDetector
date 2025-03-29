/*
		This code manages the I2C comunication between the STM32
		and the ADS1115 ADC. To start the comunication we need to
		send 2 Bytes, one containing the address and the WRITE mode,
		the other the address of the register we want to access.
		
		Then, when we again start the comunication, we will be able 
		to access the register selected with the previous frame, 
		either in write mode (if we choose the ADS1115_CONFIG_REGISTER)
		or in read mode (if we choose the ADS1115_CONVERSION_REGISTER)
		
		Either way those are 16 bits register so we need to set up 
		our frame to receive 2 Bytes.
		
		The device will be configured in continous conversion mode, one
		conversion for the I and one conversion for the Q signal.
		The STM32 will read these values once the ADC triggers an interrupt
		on EXTI4 line, assigned to pin TBD.


*/

#ifndef ADS1115_I2C_DRIVER_H
#define ADS1115_I2C_DRIVER_H

#include "stm32f3xx_hal.h"

#define ADS1115_I2C_ADDR	(0x90 <<1)		// ADDR pin to ground

#define ADS1115_ID	0

#define ADS1115_CONVERSION_REGISTER	0
#define ADS1115_CONFIG_REGISTER			1
#define ADS1115_LO_THRESH_REGISTER	2
#define ADS1115_HI_THRESH_REGISTER	3

typedef struct{

	I2C_HandleTypeDef *i2cHandle;
	int InPhase, Quadrature;
	
} ADS1115;

void ADS1115Init(ADS1115 *device, I2C_HandleTypeDef *i2cHandle);

/* ********** LOW LEVEL FUNCTIONS ********** */
HAL_StatusTypeDef ADS1115readRegister(ADS1115 *device, uint8_t reg, uint8_t *data);
HAL_StatusTypeDef ADS1115writeRegister(ADS1115 *device, uint8_t reg, uint8_t *data);


#endif