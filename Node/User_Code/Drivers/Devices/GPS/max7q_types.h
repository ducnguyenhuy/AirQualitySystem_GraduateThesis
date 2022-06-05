/*!
 * \file      max7q_types.h
 *
 * \brief     max7q driver types
 *
 * \copyright @SANSLAB
 *
 * \code
 *              _____         _   _  _____ _               ____  
 *             / ____|  /\   | \ | |/ ____| |        /\   |  _ \ 
 *            | (___   /  \  |  \| | (___ | |       /  \  | |_) |
 *             \___ \ / /\ \ | . ` |\___ \| |      / /\ \ |  _ < 
 *             ____) / ____ \| |\  |____) | |____ / ____ \| |_) |
 *            |_____/_/    \_\_| \_|_____/|______/_/    \_\____/ 
 *            (C)Smart Applications and Network System Laboratory
 *
 * \endcode
 *
 * \author    Trung Ha Nguyen ( Sanslab )
 */
 
#ifndef MAX7Q_TYPES_H
#define MAX7Q_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"
#include "Std_Types.h"

#define Max7q_HUARTTypeDef    UART_HandleTypeDef
	
#define Max7q_GPIOTypeDef     GPIO_TypeDef

typedef HAL_StatusTypeDef (*HUART_Max7q_Receive_IT)(Max7q_HUARTTypeDef *huart, \
                                              uint8_t *pRxData, uint16_t Size);

typedef HAL_StatusTypeDef (*GPIO_Max7q_Write)(Max7q_GPIOTypeDef *GPIOx, \
                                              uint16_t GPIO_Pin, GPIO_PinState PinState);									   
typedef struct 
{
	
	HUART_Max7q_Receive_IT uart_receive_it;
	GPIO_Max7q_Write       gpio_write;     
	
}Max7q_CfgType;	

											 
typedef struct
{
	uint8_t			UTC_Hour;
	uint8_t			UTC_Min;
	uint8_t			UTC_Sec;
	uint16_t		UTC_MicroSec;
	
	float			Latitude;
	double			LatitudeDecimal;
	char			NS_Indicator;
	float			Longitude;
	double			LongitudeDecimal;
	char			EW_Indicator;
	
	uint8_t			PositionFixIndicator;
	uint8_t			SatellitesUsed;
	float			HDOP;
	float			MSL_Altitude;
	char			MSL_Units;
	float			Geoid_Separation;
	char			Geoid_Units;
	
	uint16_t		AgeofDiffCorr;
	char			DiffRefStationID[4];
	char			CheckSum[2];	
	
}GPGGA_t;

typedef struct 
{
	uint8_t		rxBuffer[512];
	uint16_t	rxIndex;
	uint8_t		rxTmp;	
	uint32_t	LastTime;	
	
	GPGGA_t		GPGGA;
	
}GPS_t;

/*!
 * Max7q GPIO
 */
typedef struct
{
	Max7q_GPIOTypeDef  *gpio_port;
	uint16_t            gpio_pin;
} Max7q_Gpio_t;


typedef struct
{
	Max7q_HUARTTypeDef   *huart;        ///< HUART HAL Instance        
	Max7q_Gpio_t         reset_pin;     ///< Reset pin
	Max7q_Gpio_t         exti_pin;      ///< EXTI pin
	GPS_t                gps;           ///< Max7q gps    
} Max7q_t;

#ifdef __cplusplus
}
#endif
#endif /* MAX7Q_TYPES_H */