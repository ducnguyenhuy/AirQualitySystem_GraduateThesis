#include "GPS_Services.h"
#include "max7q.h"
#include "main.h"

extern Max7q_t max7q;

static void GPS_Service (void const * arg);

void GPS_StartService(osThreadId *Service_ID)
{
	if(*Service_ID == NULL)
	{
		osThreadDef(GPS_SV, GPS_Service, osPriorityNormal, 0, 512);
		*Service_ID = osThreadCreate(osThread(GPS_SV), NULL);
	}
}

static void GPS_Service (void const * arg)
{
	Max7q_Init(&max7q);
	
	while(1)
	{
		Max7q_Process(&max7q);
		osDelay(100);
	}
}