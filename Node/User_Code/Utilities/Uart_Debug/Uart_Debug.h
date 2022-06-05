#ifndef Uart_DEBUG_H
#define Uart_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include "stm32l0xx_hal.h"
#include "./cfg/Uart_Debug_Cfg.h"

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
  #define GETCHAR_PROTOTYPE int fgetc(FILE *f)
	#define DEBUG             printf
#endif /* __GNUC__ */

#ifdef __cplusplus
}
#endif

#endif /* UART_DEBUG_H */
