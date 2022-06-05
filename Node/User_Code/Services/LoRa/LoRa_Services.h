#ifndef LORA_SERVICES_H
#define LORA_SERVICES_H

#ifdef __cplusplus
extern "C" {
#endif
#include "Std_Types.h"
#include "main.h"

#define Eb_HUARTTypeDef    UART_HandleTypeDef
	
#define Eb_GPIOTypeDef     GPIO_TypeDef

#define Eb_Stt_Port        GPIOA

#define Eb_Stt_Pin         GPIO_PIN_2

#define EB_RX_BUFFER_SIZE  50
/*!
 * Max7q GPIO
 */
typedef struct
{
	Eb_GPIOTypeDef  *gpio_port;
	uint16_t            gpio_pin;
} Eb_Gpio_t;

typedef struct 
{
	Eb_HUARTTypeDef *huart;
	Eb_Gpio_t       stt_pin;
	
	uint8_t		rxBuffer[EB_RX_BUFFER_SIZE];
	uint16_t	rxIndex;
	uint8_t		rxTmp[2];	
	uint8_t   cpltUartdata;
	uint16_t  dataLen;
} ExtBoard_t;


extern ExtBoard_t detector_board;

//void LoRa_StartService(osThreadId *Service_ID);

//void LoRa_CloseService(osThreadId *Service_ID);

void LoRa_Process(void);

#ifdef __cplusplus
}
#endif
#endif /* LORA_SERVICES_H */
