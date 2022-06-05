#ifndef GPS_SERVICES_H
#define GPS_SERVICES_H

#ifdef __cplusplus
extern "C" {
#endif
#include "Std_Types.h"
	
void GPS_StartService(osThreadId *Service_ID);
	
#ifdef __cplusplus
}
#endif
#endif /* GPS_SERVICES_H */