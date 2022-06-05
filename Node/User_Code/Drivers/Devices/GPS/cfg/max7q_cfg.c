#include "usart.h"
#include "./cfg/max7q_cfg.h"

extern UART_HandleTypeDef MAX7Q_HUART;

Max7q_CfgType max7q_cmd ={
	
	(void *) HAL_UART_Receive_IT,
	(void *) HAL_GPIO_WritePin,
};

Max7q_t max7q = {
	&MAX7Q_HUART, \
	{MAX7Q_RST_PORT, MAX7Q_RST_PIN}, \
	{MAX7Q_EXTI_PORT, MAX7Q_EXTI_PIN}, \
};

