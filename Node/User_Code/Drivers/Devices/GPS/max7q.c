#include "max7q.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Uart_Debug.h"

/*
 * Private functions prototypes
 */
 
/*!
 * \brief Convert Degree Minute to decimal Degree
 *
 * \param [IN] degMin
 * 
 * \retval decDeg
 */ 
static double convertDegMinToDecDeg (float degMin);

/*
 * Max7q driver functions implementation
 */
 
static double convertDegMinToDecDeg (float degMin)
{
  double min = 0.0;
  double decDeg = 0.0;
 
  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);
 
  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );
 
  return decDeg;
}

void Max7q_Init(Max7q_t *max7q)
{
	max7q->gps.rxIndex = 0;
	max7q_cmd.gpio_write(max7q->reset_pin.gpio_port,max7q->reset_pin.gpio_pin,1);
	max7q_cmd.uart_receive_it(max7q->huart,&max7q->gps.rxTmp,1);
}

void Max7q_CallBack(Max7q_t *max7q)
{
	max7q->gps.LastTime = HAL_GetTick();
	if(max7q->gps.rxIndex < sizeof(max7q->gps.rxBuffer)-2)
	{
		max7q->gps.rxBuffer[max7q->gps.rxIndex] = max7q->gps.rxTmp;
		max7q->gps.rxIndex++;
	}
	max7q_cmd.uart_receive_it(max7q->huart,&max7q->gps.rxTmp,1);
}

void Max7q_Process(Max7q_t *max7q)
{
	if((HAL_GetTick()-max7q->gps.LastTime>50) && (max7q->gps.rxIndex>0))
	{
		char	*str;
		#if (MAX7Q_DEBUG==1)
		DEBUG_USER("GPS INFOR\r\n%s",max7q->gps.rxBuffer);
		#endif
		str=strstr((char*)max7q->gps.rxBuffer,"$GPGGA,");
		if(str!=NULL)
		{
			memset(&max7q->gps.GPGGA,0,sizeof(max7q->gps.GPGGA));
			sscanf(str,"$GPGGA,%2hhd%2hhd%2hhd.%3hd,%f,%c,%f,%c,%hhd,%hhd,%f,%f,%c,%hd,%s,*%2s\r\n",&max7q->gps.GPGGA.UTC_Hour, \
			                                                                                        &max7q->gps.GPGGA.UTC_Min, \
																									                                            &max7q->gps.GPGGA.UTC_Sec, \
                                                                                              &max7q->gps.GPGGA.UTC_MicroSec, \
                                                                                              &max7q->gps.GPGGA.Latitude, \
                                                                                              &max7q->gps.GPGGA.NS_Indicator, \
                                                                                              &max7q->gps.GPGGA.Longitude, \
                                                                                              &max7q->gps.GPGGA.EW_Indicator, \
                                                                                              &max7q->gps.GPGGA.PositionFixIndicator, \
                                                                                              &max7q->gps.GPGGA.SatellitesUsed, \
                                                                                              &max7q->gps.GPGGA.HDOP, \
                                                                                              &max7q->gps.GPGGA.MSL_Altitude, \
                                                                                              &max7q->gps.GPGGA.MSL_Units, \
                                                                                              &max7q->gps.GPGGA.AgeofDiffCorr, \
                                                                                              max7q->gps.GPGGA.DiffRefStationID, \
                                                                                              max7q->gps.GPGGA.CheckSum);
			if(max7q->gps.GPGGA.NS_Indicator==0)
				max7q->gps.GPGGA.NS_Indicator='-';
			if(max7q->gps.GPGGA.EW_Indicator==0)
				max7q->gps.GPGGA.EW_Indicator='-';
			if(max7q->gps.GPGGA.Geoid_Units==0)
				max7q->gps.GPGGA.Geoid_Units='-';
			if(max7q->gps.GPGGA.MSL_Units==0)
				max7q->gps.GPGGA.MSL_Units='-';
			if((max7q->gps.GPGGA.Latitude!=0) && (max7q->gps.GPGGA.Longitude != 0))
			{
				max7q->gps.GPGGA.LatitudeDecimal=convertDegMinToDecDeg(max7q->gps.GPGGA.Latitude);
				max7q->gps.GPGGA.LongitudeDecimal=convertDegMinToDecDeg(max7q->gps.GPGGA.Longitude);			
			}
		}		
		memset(max7q->gps.rxBuffer,0,sizeof(max7q->gps.rxBuffer));
		max7q->gps.rxIndex=0;
	}
	max7q_cmd.uart_receive_it(max7q->huart,&max7q->gps.rxTmp,1);
}


