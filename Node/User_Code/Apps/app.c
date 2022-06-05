#include "app.h"
#include "LoRa_Services.h"
//#include "GPS_Services.h"
#include "Uart_Debug.h"

#include "sx127x.h"
extern Sx127x_t sx127x_node;

extern ExtBoard_t detector_board;


void enter_Standby( void )
{
    /* Enable Clocks */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
     
    /* Prepare for Standby */
    // if WKUP pins are already high, the WUF bit will be set
    PWR->CSR |= PWR_CSR_EWUP1 | PWR_CSR_EWUP2;
     
    PWR->CR |= PWR_CR_CWUF; // clear the WUF flag after 2 clock cycles
    PWR->CR |= PWR_CR_ULP;   // V_{REFINT} is off in low-power mode
    PWR->CR |= PWR_CR_PDDS; // Enter Standby mode when the CPU enters deepsleep
     
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; // low-power mode = stop mode
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk; // reenter low-power mode after ISR
    __WFI(); // enter low-power mode
}

void LoRa_Test()
{
	//HAL_GPIO_WritePin (sx127x_node.reset_pin.gpio_port,sx127x_node.reset_pin.gpio_pin,GPIO_PIN_SET);
	Sx127x_Reset(&sx127x_node);
  Sx127x_SetOpMode(&sx127x_node,  0x80| SX127X_MODE_SLEEP);
	uint8_t reg_val =0;
////	Sx127x_SetModeStdby(&sx127x_node);

	Sx127x_ReadReg(&sx127x_node,SX127X_REG_01_OP_MODE, &reg_val);
	DEBUG_USER("%u\r\n",reg_val);
////	if( (reg_val & ~SX127X_OPMODE_MASK ) == 0x00 )
////  {
////		DEBUG_USER("YES\r\n");
////  }
	Sx127x_WriteReg(&sx127x_node, SX127X_REG_01_OP_MODE, (reg_val & ~SX127X_OPMODE_MASK) | SX127X_MODE_SLEEP);
	
	Sx127x_ReadReg(&sx127x_node,SX127X_REG_01_OP_MODE, &reg_val);
//	if( (reg_val & ~SX127X_OPMODE_MASK ) == 0x00 )
//  {
//		DEBUG_USER("YES\r\n");
//  }
	DEBUG_USER("%u\r\n",reg_val);
//	HAL_GPIO_WritePin (sx127x_node.reset_pin.gpio_port,sx127x_node.reset_pin.gpio_pin,GPIO_PIN_RESET);
//	HAL_GPIO_WritePin (sx127x_node.nss_pin.gpio_port,sx127x_node.nss_pin.gpio_pin,GPIO_PIN_SET);
	enter_Standby();
}


void App_Main(void)
{

	LoRa_Process();
	
	while(1)
	{
		
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == detector_board.huart->Instance)
	{
		if(detector_board.cpltUartdata != 1)
		{
			if(detector_board.rxTmp[0] != '\n')
		  {
				detector_board.rxBuffer[detector_board.rxIndex++] = detector_board.rxTmp[0];
			}
			else
			{
				detector_board.cpltUartdata = 1;
				detector_board.dataLen = detector_board.rxIndex+1;
				detector_board.rxIndex = 0;
			}
		}

		HAL_UART_Receive_IT(detector_board.huart,(uint8_t *)detector_board.rxTmp,1);
	}
	
//	if(huart->Instance == USART4)
//	{
//		Max7q_CallBack(&max7q);
//	}
}

