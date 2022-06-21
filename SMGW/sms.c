#include "ec2x_def.h"
#include "serialPort.h"
#include "Lte.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define DEVICE                          "/dev/ttyUSB2"

/* Write command via USB port */
static Std_ReturnType SMS_SendWriteCommand(char *pcCommand, uint32_t u32CommandLen)
{
    Std_ReturnType ret = E_NOK;
    char achResBuff[MAX_CMD_SIZE];
    memset(achResBuff, 0, MAX_CMD_SIZE);

    /* cat carriage return ("\r") into command */
    strcat(pcCommand, LTE_CARRIAGE_RETURN_CHAR);
    u32CommandLen += 1;

    printf("Write command [%.*s]\n", u32CommandLen - 1, pcCommand);

    /* Write to device file */
    ret = SerialPort_Write(pcCommand, u32CommandLen);
    sleep(1);
    if (E_OK == ret)
    {
        int position = 0;
        do
        {
            SerialPort_Read(&achResBuff[position], MAX_CMD_SIZE);
            position = strlen(achResBuff);
        } while ((strstr(achResBuff, "OK") != NULL) && (strstr(achResBuff, "ERROR") != NULL));

        printf("Response of write command %s\n", achResBuff);
    }
    else
    {

        printf("Write [%.*s] is fail\n", u32CommandLen - 1, pcCommand);

    }
    return ret;
}

void send_to_uart(char *cmd)
{
    char acCommand[200];
    uint8_t u8CommandLen;
    
    memset(acCommand, 0, 200);
    sprintf(acCommand, "%s\r\n", cmd);

    u8CommandLen = strlen(acCommand);
    SMS_SendWriteCommand((uint8_t *)acCommand, u8CommandLen);
}

void send_sms(char *msg, char *phoneNumber)
{
    char bufferPhone[30];
    sprintf(bufferPhone, "AT+CMGS=\"+%s\"", phoneNumber);
    
    send_to_uart("AT");
    usleep(100000);

    send_to_uart("AT+CMGF=1");
    usleep(100000);

    send_to_uart(bufferPhone);
    usleep(100000);

    send_to_uart(msg);
    usleep(100000);

    send_to_uart("\x1A");
    usleep(100000);
}

int main()
{
    char data[255];
    int i = 0;
    char phone[30] = "84868050103";
    Std_ReturnType ret = 0;
    char outStr[255];

    SerialPort_TermiosConfigType config =
    {
        .bParityBitEnable = false,
        .bStopBitEnable = true,
        .bFlowControlEnable = false,
        .u32BaudRate = 115200
    };

    ret = SerialPort_Open((char *)DEVICE);

    SerialPort_ConfigureTermios(&config);

    send_sms("Hello", phone);
 
    printf("Starting sms ...\n");
    while(1)
    {
        
    }
    return 0;
}
