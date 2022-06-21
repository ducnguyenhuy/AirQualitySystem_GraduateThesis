#include "ec2x_def.h"
#include "serialPort.h"
#include "Lte.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>
#include <json-c/json.h>

#define DEVICE                          "/dev/ttyUSB2"

#define PATH_TO_DATA_NODE "/tmp/aqi_and_concentration.json"
#define PATH_TO_PHONE_NODE "/tmp/phone_vs_node.json"

typedef struct
{
    /* data */
    int addr;
    float aqi;
    float pm2_5;
    float co;
} data_of_node_t;

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

data_of_node_t parse_data(char *PATH_TO_DATA, int node_addr)
{
    int len, i;
    data_of_node_t node_info;

    /* Initialize address value in case of node not exist*/
    node_info.addr = 0;

    json_object *root = json_object_from_file(PATH_TO_DATA);
    json_object *addr, *node;
    json_object_object_get_ex(root, "node", &node);

    len = json_object_array_length(node);

    json_object *tmp;

    for (i = 0; i < len; i++)
    {
        tmp = json_object_array_get_idx(node, i);
        json_object_object_get_ex(tmp, "address", &addr);
        if (json_object_get_int(addr) == node_addr)
        {
            node_info.addr = node_addr;
            json_object *aqi, *pm2_5, *co;
            /* Get aqi param */
            json_object_object_get_ex(tmp, "aqi", &aqi);
            node_info.aqi = json_object_get_double(aqi);

            /* Get PM2.5 param */
            json_object_object_get_ex(tmp, "pm2_5", &pm2_5);
            node_info.pm2_5 = json_object_get_double(pm2_5);

            /* Get CO param */
            json_object_object_get_ex(tmp, "co", &co);
            node_info.co = json_object_get_double(co);

            return node_info;
        }
    }
    /* Can't find node with node_adrr */
    return node_info;
}

int send_aqi(json_object *phone_and_node)
{
    int len, j;
    char msg[60];
    data_of_node_t data_to_send;
    json_object *node_addr, *phone_arr, *num_arr, *number;

    /* Get node address*/
    json_object_object_get_ex(phone_and_node, "node_addr", &node_addr);
    data_to_send = parse_data(PATH_TO_DATA_NODE, json_object_get_int(node_addr));

    if (data_to_send.addr == 0)
    {
        printf("Node does't exist!\n");
        return -1;
    }
    /* Get phone number array */
    json_object_object_get_ex(phone_and_node, "phone", &phone_arr);

    len = json_object_array_length(phone_arr);

    for (j = 0; j < len; j++)
    {
        /* Number j*/
        num_arr = json_object_array_get_idx(phone_arr, j);
        json_object_object_get_ex(num_arr, "num", &number);

        sprintf(msg, "The AQI is %.2f", data_to_send.aqi);
        send_sms(msg, json_object_get_string(number));
    }
}

int main(void)
{
    /* Initialize port to control module EC25 */
    Std_ReturnType ret = 0;

    SerialPort_TermiosConfigType config =
        {
            .bParityBitEnable = false,
            .bStopBitEnable = true,
            .bFlowControlEnable = false,
            .u32BaudRate = 115200
        };

    ret = SerialPort_Open((char *)DEVICE);

    SerialPort_ConfigureTermios(&config);

    /* Read file json and send data */
    int lenPair, i, j;

    data_of_node_t data;

    json_object *root = json_object_from_file(PATH_TO_PHONE_NODE);
    json_object *phone_and_node;
    json_object_object_get_ex(root, "phone_and_node_addr", &phone_and_node);

    lenPair = json_object_array_length(phone_and_node);

    json_object *tmp;

    for (i = 0; i < lenPair; i++)
    {
        /* Node address and phone of number i */
        tmp = json_object_array_get_idx(phone_and_node, i);
        send_aqi(tmp);
    }

    /* Free object */
    json_object_put(root);
    json_object_put(tmp);
    return 0;
}
