#include "sensor.h"

void delay_us (uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim7,0);  // set the counter value a 0
    while (__HAL_TIM_GET_COUNTER(&htim7) < us);  // wait for the counter to reach the us input in the parameter
}



uint32_t gp2y_adc_measure(void)
{
	/**
	 * This function defines ADC channel and returns converted value
	 * NOTE: function works only with modified library "stm32l0xx_hal_adc.c" from this repository.
	**/
		uint16_t val=0;
		ADC_ChannelConfTypeDef sConfig;
		sConfig.Channel = ADC_CHANNEL_10;
		sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
		HAL_ADC_ConfigChannel(&hadc, &sConfig); 
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc,100);
		
		// Set PC1 - CONTROL PIN
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0); 
		// Delay 280us
		delay_us(280);
		// Get ADC value
		val = (uint32_t) HAL_ADC_GetValue(&hadc);
		// Delay 40us
		delay_us(40);
		// Reset PC1 - CONTROL PIN
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1); 
		delay_us(9680);

		HAL_ADC_Stop(&hadc);
		HAL_Delay(50);
		return val;
}

uint32_t mq135_adc_measure(void)
{
/**
 * This function defines ADC channel and returns converted value
 * NOTE: function works only with modified library "stm32l0xx_hal_adc.c" from this repository.
**/
	uint16_t val=0;
	ADC_ChannelConfTypeDef sConfig;
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  HAL_ADC_ConfigChannel(&hadc, &sConfig); 
	HAL_ADC_Start(&hadc);
  HAL_ADC_PollForConversion(&hadc,100);
	
	val = (uint32_t) HAL_ADC_GetValue(&hadc);
	
	HAL_ADC_Stop(&hadc);
  HAL_Delay(50);
	return val;
}


