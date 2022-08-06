#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "LoRa_Services.h"
#include "main.h"
#include "sx127x.h"
#include "Uart_Debug.h"
#include "LoRaMac_Node.h"
#include "app.h"

#include "sensor.h"

/* global variable */
extern float mq135Value;
extern float gp2yValue;

#define EEPROM_MSB_ADDR 0x08080000

//#define USING_868
//#define USING_433
#define USING_923

#define REQUIRE_ACK

#define RX_DUTY_TIME_MS            2000

#define TX_BUFFFER_SIZE            50
#define RX_BUFFFER_SIZE            50

#ifdef USING_868
	#define MAX_CHANNEL                4
	float f_channel[4] = { \
	869.525, \
	868.1,868.3,868.5};
#endif
	
#ifdef USING_433
	#define MAX_CHANNEL                2
	float f_channel[2] = { \
	435.0, \
	433.0};
#endif
	
#ifdef USING_923
	#define MAX_CHANNEL                8
	float f_channel[MAX_CHANNEL] = { \
	924.6, \
	923.2,923.4,923.6,923.8,924.0,924.2,924.4};	
#endif

extern RNG_HandleTypeDef hrng;

ExtBoard_t detector_board = {
	&hlpuart1, \
	{Eb_Stt_Port,Eb_Stt_Pin}, \
};

#define RAND_LOCAL_MAX 2147483647L

uint32_t rnd_seed = 0; // random seed

static uint32_t next = 1;

uint8_t rnd_fchannel = 0;

typedef enum LoRaSV_PROCESS_STATE_s
{
	LoRaSV_STATE_SLEEP = 0x00, \
	LoRaSV_STATE_TX, \
	LoRaSV_STATE_RX
	
}LoRaSV_PROCESS_STATE_t;


uint8_t rx_buffer[50];    // Length of rx buffer

uint8_t tx_buffer[TX_BUFFFER_SIZE];    // Length of tx buffer

uint8_t state_init = 0;   // Init of the state or not?

uint16_t cur_fcnt =0;     // Current frame cnt variable


LoRaSV_PROCESS_STATE_t lps = LoRaSV_STATE_SLEEP;

extern Sx127x_t sx127x_node;

extern LoRaMacMessageData_t tx_msg; // Tx Msg Instance

//extern CircularBuffer_t rx_lora_cirbuf; // Circular Buffer Instance for Rx

/* ====================================================================== */

static void hal_sleep(void);

static Std_ReturnType LoRaNodeSV_SendData_Normal
(
 Sx127x_t *sx127x, const uint8_t* data, \
 uint16_t len, uint16_t timeout
);
 
static Std_ReturnType LoRaNodeSV_SendData_CSMA
(
 Sx127x_t *sx127x, const uint8_t* data, \
 uint16_t len, uint16_t backoff_time,\
 uint16_t tx_timeout, uint16_t cad_timeout
);

static Std_ReturnType LoRaNodeSV_ReceiveData_Normal
(
 Sx127x_t *sx127x, uint8_t *rx_buffer, \
 uint8_t *len_rx, uint16_t timeout
);

static void LoRa_Service(void const * arg);
	
static void LoRa_MainProcess(void);

static Std_ReturnType LoRaSV_RxHandle(uint8_t* rx_buffer, uint8_t len_rx);

static Std_ReturnType LoRaSV_SendACK(void);

static void DetectorBuff_Reset(void);

static uint32_t flash_read(uint32_t address);

static void flash_write(uint32_t address, uint32_t data);
/* ====================================================================== */

int32_t rand1( void )
{
    return ( ( next = next * 1103515245L + 12345L ) % RAND_LOCAL_MAX );
}

void srand1( uint32_t seed )
{
    next = seed;
}
// Standard random functions redefinition end

int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand1( ) % ( max - min + 1 ) + min;
}

static uint32_t flash_read(uint32_t address){
    return *(uint32_t*)address;
}

static void flash_write(uint32_t address, uint32_t data){
    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,address,data);
    HAL_FLASH_Lock();
}
//void os_idle_demon (void) {
//  /* The idle demon is a system thread, running when no other thread is       */
//  /* ready to run.                                                            */
// 
//  for (;;) {
//    __WFE();                            /* Enter sleep mode                   */
//  }
//}

static void DetectorBuff_Reset(void)
{    
	detector_board.cpltUartdata = 0;
	detector_board.rxIndex = 0;
	detector_board.dataLen = 0;
	memset(detector_board.rxTmp,'\0',2);
	memset(detector_board.rxBuffer,'\0',20);
}

static Std_ReturnType LoRaNodeSV_SendData_Normal(Sx127x_t *sx127x, const uint8_t* data, uint16_t len, uint16_t timeout)
{
	Std_ReturnType ret = STD_NOT_OK;
	
	ret = Sx127x_SendData(sx127x, data, len, timeout);
	if(ret == STD_NOT_OK)
	{
		return ret;
	}
	
	return STD_OK;
}

static Std_ReturnType LoRaNodeSV_SendData_CSMA(Sx127x_t *sx127x, const uint8_t* data, uint16_t len, uint16_t backoff_time, uint16_t tx_timeout, uint16_t cad_timeout)
{
	Std_ReturnType ret = STD_NOT_OK;
	
	ret = Sx127x_Wait4FreeChannel(sx127x,10,cad_timeout);
	if(ret == STD_NOT_OK)
	{
		return ret;
	}
	
	ret = Sx127x_SendData(sx127x,data, len, tx_timeout);
	if(ret == STD_NOT_OK)
	{
		return ret;
	}
	
	return STD_OK;
}

static Std_ReturnType LoRaNodeSV_ReceiveData_Normal(Sx127x_t *sx127x, uint8_t *rx_buffer, uint8_t *len_rx, uint16_t timeout)
{
	Std_ReturnType ret = STD_NOT_OK;
	
	ret = Sx127x_ReceiveData(sx127x, rx_buffer, len_rx, timeout);
	if(ret == STD_NOT_OK)
	{
		return ret;
	}
	
	return STD_OK;
}

static Std_ReturnType LoRaSV_SendACK(void)
{
	Std_ReturnType ret = STD_NOT_OK;
	
	return STD_OK;
}
/* RxPacket Handle */
static Std_ReturnType LoRaSV_RxHandle(uint8_t* rx_buffer, uint8_t len_rx)
{
	Std_ReturnType ret = STD_NOT_OK;

	LoRaMacMessageData_t macMsgData = {0};

	uint8_t pktHeaderLen = 0;
	
	for(uint8_t i = 0; i < len_rx; i++)
	{
		macMsgData.Buffer[i] = rx_buffer[i];
	}
	
	macMsgData.BufSize = len_rx;
	
	ret = LoRaMac_ParserData(&macMsgData);
	if(ret == STD_NOT_OK)
	{
		return ret;
	}
	DEBUG_USER("%u\r\n", macMsgData.FHDR.DevAddr);
	if(macMsgData.FHDR.DevAddr != DEV_ADDR)
	{
		return STD_NOT_OK;
	}
	
	if(macMsgData.FHDR.FCnt != cur_fcnt)
	{
		return STD_NOT_OK;
	}
	
	if(macMsgData.FHDR.FCtrl.Bits.Ack != 1)
	{
		return STD_NOT_OK;
	}
	DEBUG_USER("ACK is received\r\n");
	
	/* Handle command here */
	switch(macMsgData.MHDR.Bits.MType)
	{
		case FRAME_TYPE_COMMAND_UNCONFIRMED:
		{
			
			break;
		}
		case FRAME_TYPE_COMMAND_CONFIRMED:
		{
			LoRaMacMessageData_t *ack_msg;
			// Sending ACK
			LoRaMac_SetUpACKMessage(ack_msg,macMsgData.FHDR.FCnt);
			
			/* Change to uplink frequency to transmit msg to GW*/
			ret = Sx127x_SetModeStdby(&sx127x_node);
			
			/* Random frequency */
			HAL_RNG_GenerateRandomNumber(&hrng,&rnd_seed); // Generating a 32bit random number for random seed
			srand1(rnd_seed); // Init seed
			rnd_fchannel = randr(1,MAX_CHANNEL-1); // Random a number for delay time
			DEBUG_USER("TxFreq : %f\r\n",f_channel[rnd_fchannel]);
			
			ret = Sx127x_SetFrequency(&sx127x_node,f_channel[rnd_fchannel]);
			if(ret == STD_NOT_OK)
			{
				DEBUG_USER("SetULfreqFAIL\r\n");
			}
			else
			{
				DEBUG_USER("SetULfreqOK\r\n");
			}
			
			ret = LoRaNodeSV_SendData_Normal(&sx127x_node, ack_msg->Buffer, ack_msg->BufSize, SX127X_TX_TIMEOUT_MS);
			while(ret == STD_NOT_OK )
			{
				DEBUG_USER("TxACKFail\r\n");
				ret = LoRaNodeSV_SendData_Normal(&sx127x_node, ack_msg->Buffer, ack_msg->BufSize, SX127X_TX_TIMEOUT_MS);
			}

			DEBUG_USER("TxACKDone\r\n");
			break;
		}
		default:
		{
			break;
		}
	}
	
	return STD_OK;
}

//void LoRa_StartService(osThreadId *Service_ID)
//{
//	if(*Service_ID == NULL)
//	{
//		osThreadDef(LoRa_SV, LoRa_Service, osPriorityNormal, 0, 512);
//		*Service_ID = osThreadCreate(osThread(LoRa_SV), NULL);
//	}
//}

//void LoRa_CloseService(osThreadId *Service_ID)
//{
//	if(*Service_ID != NULL)
//	{
//		osThreadTerminate(*Service_ID);
//		*Service_ID = NULL;
//	}
//}
////////////////////////////////////////////////
static void Enter_SleepMode(void);

static void enter_Standby(void);

void LoRa_Process(void)
{
	LoRa_Service(NULL);
}
static void Enter_SleepMode(void)
{
	HAL_SuspendTick();
	
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON,PWR_SLEEPENTRY_WFI);
	//HAL_PWR_EnterSTANDBYMode();
	//HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	
	HAL_ResumeTick();
}

// Enter STBY Mode function. 
static void enter_Standby( void )
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
////////////////////////////////////////////////
static void LoRa_Service(void const * arg)
{
	Std_ReturnType ret = STD_NOT_OK;
	
	DEBUG_USER("\r\n=======Start LoRa Service=======\r\n");
	
	Sx127x_Reset(&sx127x_node);

	ret = Sx127x_InitModule(&sx127x_node);
	if(ret == STD_NOT_OK)
	{
		DEBUG_USER("Init LoRa Fail\r\n");
	}
	else
	{
		DEBUG_USER("Init LoRa OK\r\n");
	}
	
	LoRa_MainProcess();
	
	while(1)
	{
		
	}
}

static void LoRa_MainProcess(void)
{
	Std_ReturnType ret = STD_NOT_OK;
	
	LoRaMacSerializerStatus_t serialize_ret = LORAMAC_SERIALIZER_ERROR;
	
	uint32_t time_stamp =  0;
	
	uint8_t retry = 0;
	
	uint16_t pre_fcnt = 0;
	
	uint8_t rxlen = 0;       // Length of rx data
	
	state_init = 1;
	
	lps = LoRaSV_STATE_TX;
	
	DetectorBuff_Reset();
	
	HAL_UART_Receive_IT(detector_board.huart,(uint8_t *)detector_board.rxTmp,1); // Enable uart receive interupt
	
	uint8_t u8Count = 0; 
	while(1)
	{
		switch(lps)
		{
			case LoRaSV_STATE_TX:
			{
				DEBUG_USER("\r\n=======TX=======\r\n");
				HAL_GPIO_WritePin(detector_board.stt_pin.gpio_port,detector_board.stt_pin.gpio_pin,GPIO_PIN_SET);
				/* Check if device in retrying transmiting */
				if(!retry)
				{
					u8Count = 0;
					/* Informing ready state to the uart device  */
					

					/* Timeout for receiving data from other uart device*/
					//detector_board.cpltUartdata = 1;
					
					
//					time_stamp = HAL_GetTick();
//					while (((HAL_GetTick() - time_stamp) < 5000) && (!detector_board.cpltUartdata));
//					if(!detector_board.cpltUartdata)
//					{
//						lps = LoRaSV_STATE_SLEEP;
//						break;
//					}
				}
				else
				{
					if(u8Count++ == 3)
					{
						u8Count = 0;
						lps = LoRaSV_STATE_SLEEP;
						
					}
				}
				
				/* Change to uplink frequency to transmit msg to GW*/
				ret = Sx127x_SetModeStdby(&sx127x_node);
				
				HAL_RNG_GenerateRandomNumber(&hrng,&rnd_seed); // Generating a 32bit random number for random seed
			  srand1(rnd_seed); // Init seed
			  rnd_fchannel = randr(1,MAX_CHANNEL-1); // Random a number for frequency slot
			  DEBUG_USER("TxFreq : %f\r\n",f_channel[rnd_fchannel]);
				
				ret = Sx127x_SetFrequency(&sx127x_node,f_channel[rnd_fchannel]);
				if(ret == STD_NOT_OK)
				{
					DEBUG_USER("SetULfreqFAIL\r\n");
				}
				else
				{
					DEBUG_USER("SetULfreqOK\r\n");
				}
				
				/* Set up data to transmit */
				//memcpy(tx_buffer, detector_board.rxBuffer, detector_board.dataLen);
				
//				for (int k = 0; k < detector_board.dataLen; k++)
//				{
//					tx_buffer[k] = detector_board.rxBuffer[k];
//					DEBUG_USER("%c", tx_buffer[k]);
//				}
//				DEBUG_USER("\r\n");
				
				//========================================= Code by DucNH =======================================
				char cmd_send[50];
				
				
			
				
				sprintf(cmd_send, "%f, %f\r\n", gp2yValue, mq135Value);
				
				detector_board.dataLen = 45;
				memcpy(tx_buffer, cmd_send, detector_board.dataLen);
				DEBUG_USER("%s\r\n",tx_buffer);
				/* Get Counter from Flash */
				cur_fcnt = flash_read(EEPROM_MSB_ADDR);
				
				serialize_ret = LoRaMac_SetUpDataMessage(&tx_msg, STD_OFF, 0, &cur_fcnt, tx_buffer, detector_board.dataLen);
				if(serialize_ret != LORAMAC_SERIALIZER_SUCCESS)
				{
					DEBUG_USER("serializeFAIL\r\n");
				}
				else
				{
					DEBUG_USER("serializeOK\r\n");
				}
				
				ret = LoRaNodeSV_SendData_Normal(&sx127x_node, tx_msg.Buffer, tx_msg.BufSize, SX127X_TX_TIMEOUT_MS);
				
				if(ret == STD_OK )
				{
					DEBUG_USER("TxDone\r\n");
					/* Change to Rx state */
					lps = LoRaSV_STATE_RX;
					state_init = 1;
					retry = 0;
				}
				else
				{
					//<!> if Tx failed => state is remained and retry to send
					retry = 1;
					DEBUG_USER("TxFail\r\n");
				}
				// finally enter the standby mode
				DEBUG("Enter the standby mode\n");
				HAL_PWR_EnterSTANDBYMode();
				break;
			}
			case LoRaSV_STATE_RX:
			{
				/* Init Phase */
				if(state_init == 1)
				{
					DEBUG_USER("\r\n=======RX=======\r\n");
					
					/* Change to downlink frequency to receive msg from GW*/
					ret = Sx127x_SetModeStdby(&sx127x_node);
					
					ret = Sx127x_SetFrequency(&sx127x_node,f_channel[0]);
					if(ret == STD_NOT_OK)
					{
						DEBUG_USER("SetDLfreqFAIL\r\n");
					}
					else
					{
						DEBUG_USER("SetDLfreqOK\r\n");
					}
					
					/* Get time now to start Rx */
					time_stamp = HAL_GetTick(); // Get time now
					
					/* Initializing state is done */
					state_init = 0;
				}
				
				pre_fcnt = cur_fcnt;
				while ((HAL_GetTick() - time_stamp) < RX_DUTY_TIME_MS)
				{
					/* Receive Data */				
				  ret = LoRaNodeSV_ReceiveData_Normal(&sx127x_node, rx_buffer, &rxlen, SX127X_RX_TIMEOUT_MS);
					if(ret == STD_OK)
					{
						DEBUG_USER("RxOK\r\n");
						
						ret = LoRaSV_RxHandle(rx_buffer,rxlen);
						if(ret == STD_NOT_OK)
						{
							//DEBUG_USER("RxHandle Failed\r\n");
						}
						else
						{
							//DEBUG_USER("RxHandle OK\r\n");
							cur_fcnt++;
							flash_write(EEPROM_MSB_ADDR,cur_fcnt);

							break;
						}
					}
					else
					{
						DEBUG_USER("RxFail\r\n");
					}
				}

#ifdef REQUIRE_ACK		
				
				/* Check if No ACK */
				if(pre_fcnt == cur_fcnt)
				{
					DEBUG_USER("No ACK + Timeout => Retransmitting Packet %u...\r\n", cur_fcnt);
					lps = LoRaSV_STATE_TX;
					retry = 1;
					break;
				}
				
#endif	
        HAL_Delay(2000);				
				retry = 0;
				lps = LoRaSV_STATE_TX;
				
				break;
			} /* End RX State */	
			case LoRaSV_STATE_SLEEP:
			{
				DEBUG_USER("\r\n=======SLEEP=======\r\n");
				DetectorBuff_Reset();
				HAL_GPIO_WritePin(detector_board.stt_pin.gpio_port,detector_board.stt_pin.gpio_pin,GPIO_PIN_RESET);
				/* Set LoRa in sleep mode*/
				ret = Sx127x_SetModeSleep(&sx127x_node);
				if(ret == STD_NOT_OK)
				{
					DEBUG_USER("Set Sleep Fail\r\n");
				}
				else
				{
					DEBUG_USER("Set Sleep OK\r\n");
				}
				
				/* Set MCU in standby mode*/
				//enter_Standby();
					
					
			}
		}
	}
}
