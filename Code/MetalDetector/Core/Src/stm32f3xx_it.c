#include "main.h"
#include "stm32f3xx_it.h"
#include "ADC.h"

void NMI_Handler(void){while (1){}}
void HardFault_Handler(void){while (1){}}
void MemManage_Handler(void){while (1){}}
void BusFault_Handler(void){while (1){}}
void UsageFault_Handler(void){while (1){}}
void SVC_Handler(void){}
void DebugMon_Handler(void){}
void PendSV_Handler(void){}
	
volatile int conversionFlag = 1, n=0, c=0, p=0;
extern int calibrationSlow, calibrationFast;
extern int evaluateAngle;

void SysTick_Handler(void)
{
  /* SAMPLE Q AND I ALTERNATIVELY */
	if(conversionFlag == 1) conversionFlag++;
	else conversionFlag--;
	ADC1_Read(conversionFlag);
	
	/* EVALUATE PHASE AND MODULE OF RECEIVED SIGNAL IN MAIN LOOP */	
	if(n == 1000){
		evaluateAngle = 1;
		n = 0;
	} else n++;
	
	/* SLOW CALIBRATION FOR I SIGNAL */
	if(p == 1500){
		calibrationSlow = 1;
		p = 0;
	} else p++;
	
	
	/* FAST CALIBRATION FOR Q SIGNAL */
		if(c == 200){
		calibrationFast = 1;
		c = 0;
	} else c++;
	
  HAL_IncTick();
}
