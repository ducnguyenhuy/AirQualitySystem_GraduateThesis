#ifndef Uart_DEBUG_CFG_H
#define Uart_DEBUG_CFG_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "usart.h"

extern UART_HandleTypeDef huart1;
	
#define DEBUG_Port huart1
#define DEBUG_USER printf
#define DB_YES

#ifdef __cplusplus
}
#endif

#endif /* UART_DEBUG_H */
