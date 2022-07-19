#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	int secSleep;
    time_t now, timeToSendSms;
    struct tm timeToSend;

    // Get current time (now)
    now = time(NULL);

    // Copy current date to a `threepm`, and set time
    memcpy(&timeToSend, gmtime(&now), sizeof(struct tm));
    // if(threepm.tm_hour > 15) {
    //     // Advance to a next day
    //     ++threepm.tm_mday;
    // }

    timeToSend.tm_hour = 21;
    timeToSend.tm_min = 52;
	timeToSend.tm_sec = 0;

	secSleep =(difftime(mktime(&timeToSend), now));
	if(secSleep < 0)
	{
		secSleep = 86400 + secSleep;
	}
    printf("%d seconds till 3:00 PM\n", secSleep);

    return 0;
}