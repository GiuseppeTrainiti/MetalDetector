#include "ADS1115.h"

HAL_StatusTypeDef ADS1115readRegister(ADS1115 *device, uint8_t reg, uint8_t *data){
	return HAL_I2C_Mem_Read(device->i2cHandle, ADS1115_I2C_ADDR, reg, I2C_MEMADD_SIZE_16BIT, data, 2, HAL_MAX_DELAY);
}

HAL_StatusTypeDef ADS1115writeRegister(ADS1115 *device, uint8_t reg, uint8_t *data){
	return HAL_I2C_Mem_Write(device->i2cHandle, ADS1115_I2C_ADDR, reg, I2C_MEMADD_SIZE_16BIT, data, 2, HAL_MAX_DELAY);
}

void ADS1115Init(ADS1115 *device, I2C_HandleTypeDef *i2cHandle){
	device->i2cHandle = i2cHandle;
	
	device->InPhase = 0;
	device->Quadrature = 0;
}
