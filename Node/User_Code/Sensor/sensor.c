#include "sensor.h"

void delay_us (uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim7,0);  // set the counter value a 0
    while (__HAL_TIM_GET_COUNTER(&htim7) < us);  // wait for the counter to reach the us input in the parameter
}

uint16_t get_adc_mq135(void)
{
		uint16_t valMQ135 = 0;
			
		ADC_Select_MQ135();
	  HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, 1000);
	  valMQ135 = HAL_ADC_GetValue(&hadc);
	  HAL_ADC_Stop(&hadc);
	
		return valMQ135;
}


uint16_t get_adc_gp2y(void)
{
		uint16_t valGP2Y;
	
		ADC_Select_GP2Y();
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, 1000);
		
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0); 
		// Delay 280us
		delay_us(280);
		// Get ADC value
		valGP2Y = HAL_ADC_GetValue(&hadc);
		// Delay 40us
		delay_us(40);
		// Reset PC1 - CONTROL PIN
 		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1); 
		delay_us(9680);
	  HAL_ADC_Stop(&hadc);

		return valGP2Y;
}


