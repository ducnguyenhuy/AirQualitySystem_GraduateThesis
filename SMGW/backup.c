
// #include "ec2x_def.h"
// #include "serialPort.h"
// #include "Lte.h"
// #include <stdlib.h>
// #include <errno.h>
// #include <unistd.h>
// #include <string.h>
// #include <json-c/json.h>

// #define PATH_TO_PHONE_NODE          "/tmp/phone_vs_node.json"
// #define PATH_TO_DATA_NODE           "/tmp/aqi_and_concentration.json"

// #define DEVICE "/dev/ttyUSB2"

// typedef struct
// {
//     char pNum[100];
//     char cmd[100];
// } cmd_pNum_t;

// typedef struct
// {
//     /* data */
//     int addr;
//     float aqi;
//     float pm2_5;
//     float co;
// } data_of_node_t;

// typedef enum {
//     RETURN_OK = 0,
//     RETURN_FAILED,
//     ADDR_EXIST,
//     ADDR_NOT_EXIST,
//     PHONE_NUM_EXIST,
//     PHONE_NUM_NOT_EXIST
// } return_status_t;

// char respond[1024 * 1024];

// /* Write command via USB port */
// static Std_ReturnType SMS_SendWriteCommand(char *pcCommand, uint32_t u32CommandLen)
// {
//     Std_ReturnType ret = E_NOK;
//     // char achResBuff[MAX_CMD_SIZE];
//     // memset(achResBuff, 0, MAX_CMD_SIZE);

//     char achResBuff[1024 * 1024];
//     memset(achResBuff, 0, 1024 * 1024);

//     /* cat carriage return ("\r") into command */
//     strcat(pcCommand, LTE_CARRIAGE_RETURN_CHAR);
//     u32CommandLen += 1;

//     printf("Write command [%.*s]\n", u32CommandLen - 1, pcCommand);

//     /* Write to device file */
//     ret = SerialPort_Write(pcCommand, u32CommandLen);
//     sleep(1);
//     if (E_OK == ret)
//     {
//         int position = 0;
//         do
//         {
//             SerialPort_Read(&achResBuff[position], MAX_CMD_SIZE);
//             position = strlen(achResBuff);
//         } while ((strstr(achResBuff, "OK") != NULL) && (strstr(achResBuff, "ERROR") != NULL));

//         printf("Response of write command %s\n", achResBuff);
//         strcpy(respond, achResBuff);
//     }
//     else
//     {

//         printf("Write [%.*s] is fail\n", u32CommandLen - 1, pcCommand);
//     }
//     return ret;
// }

// void send_to_uart(char *cmd)
// {
//     char acCommand[200];
//     uint8_t u8CommandLen;

//     memset(acCommand, 0, 200);
//     sprintf(acCommand, "%s\r\n", cmd);

//     u8CommandLen = strlen(acCommand);
//     SMS_SendWriteCommand((uint8_t *)acCommand, u8CommandLen);

//     usleep(100000);
// }

// void send_sms(char *msg, char *phoneNumber)
// {
//     char bufferPhone[30];
//     sprintf(bufferPhone, "AT+CMGS=\"+%s\"", phoneNumber);

//     send_to_uart("AT");
//     usleep(100000);

//     send_to_uart("AT+CMGF=1");
//     usleep(100000);

//     send_to_uart(bufferPhone);
//     usleep(100000);

//     send_to_uart(msg);
//     usleep(100000);

//     send_to_uart("\x1A");
//     usleep(100000);
// }

// cmd_pNum_t parse_msg(char *msg)
// {
//     cmd_pNum_t parsedVal;
//     size_t len = strlen(msg);
//     int i, count;
//     int flag = 0;
//     count = 0;

//     if (msg == NULL)
//     {
//         return parsedVal;
//     }

//     for (i = 0; i < len; i++)
//     {
//         if (msg[i] == ',' && flag == 0)
//         {
//             memcpy(parsedVal.pNum, msg + i + 2, 12);
//             parsedVal.pNum[12] = '\0';
//             flag = 1;
//         }
//         if (msg[i] == ',')
//         {
//             count++;
//         }

//         if (count == 4)
//         {
//             memcpy(parsedVal.cmd, msg + i + 15, len - i);
//             parsedVal.cmd[len] = '\0';
//             break;
//         }
//     }
//     return parsedVal;
// }

// data_of_node_t *parse_data(char *PATH_TO_DATA, int node_addr)
// {
//    int len, i;
//    data_of_node_t *pNode_info, node_info;
//     pNode_info = &node_info;
//    /* Initialize address value in case of node not exist*/ 
//    pNode_info->addr = 0;

//    json_object *root = json_object_from_file(PATH_TO_DATA);

   
//    json_object *addr, *node;
//    json_object_object_get_ex(root, "node", &node);

//    len = json_object_array_length(node);
   
//    json_object *tmp;


//    for(i = 0; i<len; i++)
//    {
//       tmp = json_object_array_get_idx(node, i);
//       json_object_object_get_ex(tmp, "address", &addr);
//       if(json_object_get_int(addr) == node_addr)
//       {
//           pNode_info->addr = node_addr;
//           json_object *aqi, *pm2_5, *co;
          
//           /* Get aqi param */
//           json_object_object_get_ex(tmp, "aqi", &aqi);
//           pNode_info->aqi = json_object_get_double(aqi);

//           /* Get PM2.5 param */
//           json_object_object_get_ex(tmp, "pm2_5", &pm2_5);
//           pNode_info->pm2_5 = json_object_get_double(pm2_5);

//           /* Get CO param */
//           json_object_object_get_ex(tmp, "co", &co);
//           pNode_info->co = json_object_get_double(co);

//           return pNode_info;
//       }
//    }
//    /* Can't find node with node_adrr */
//    return pNode_info;
// }

// return_status_t check_phone_in_list(char *phoneNum, data_of_node_t *pBuff)
// {
//     int lenPair, lenNum, i, j;
//     int checkFlag = 0;
//     data_of_node_t *data, pdata;

//     data = &pdata;

//     /* Get list of phone number and address of the node */
//     json_object *root = json_object_from_file("/tmp/phone_vs_node.json");
//     json_object *phone_and_node;
//     json_object_object_get_ex(root, "phone_and_node_addr", &phone_and_node);

//     lenPair = json_object_array_length(phone_and_node);
//     json_object *tmp, *node_addr, *num_arr, *num, *phone_arr;

//     for (i = 0; i < lenPair; i++)
//     {
//         tmp = json_object_array_get_idx(phone_and_node, i);
//         /* Get phone number array of node i*/
//         json_object_object_get_ex(tmp, "phone", &phone_arr);

//         lenNum = json_object_array_length(phone_arr);
//         for (j = 0; j < lenNum; j++)
//         {
//             /* Number j*/
//             num_arr = json_object_array_get_idx(phone_arr, j);
//             json_object_object_get_ex(num_arr, "num", &num);

//             if(strcmp(json_object_get_string(num), phoneNum) == 0)
//             {  
//                 checkFlag = 1;
//                 break;
//             }
//         }
//         if(checkFlag)
//         {
//             checkFlag = 0;
//             /* Get node address */
//             json_object_object_get_ex(tmp, "node_addr", &node_addr);
//             data = parse_data(PATH_TO_DATA_NODE, json_object_get_int(node_addr));
                
//             pBuff = data;

//             if (data->addr == 0)
//             {
//                 printf("Node does't exist!\n");
//                 return ADDR_NOT_EXIST;
//             }
//             else {
//                 return RETURN_OK;
//             }
//         }
//     }
        
//     printf("Phone number not exist!\n");
//     return PHONE_NUM_NOT_EXIST;
// }

// // void *read_msg(void *arg)
// // {
// //     int len;
// //     cmd_pNum_t cmd_phone;
// //     // read message in text mode
// //     send_to_uart("AT+CMGF=1");
// //     while (1)
// //     {
// //         send_to_uart("AT+CMG=\"ALL\"");
// //         if (strlen(respond) == 16)
// //         {
// //             goto time_break;
// //         }
// //         send_to_uart("AT+CMGR=0");
// //         cmd_phone = parse_msg(respond);
// //         // sleep 10s before handle
// //     time_break:
// //         sleep(10);
// //     }
// // }

// int main()
// {
//     char data[255];
//     int i = 0;
//     char phone[30] = "84868050103";
//     Std_ReturnType ret = 0;
//     char outStr[255];

//     SerialPort_TermiosConfigType config =
//         {
//             .bParityBitEnable = false,
//             .bStopBitEnable = true,
//             .bFlowControlEnable = false,
//             .u32BaudRate = 115200};

//     ret = SerialPort_Open((char *)DEVICE);

//     SerialPort_ConfigureTermios(&config);

//     // find out which storages are supported 
//     send_to_uart("AT+CMGF=1");

//     send_to_uart("AT+CMGR=0");

//     cmd_pNum_t tmp;
//     data_of_node_t *pBuff, buff;
//     return_status_t return_status;

//     pBuff = &buff;

//     tmp = parse_msg(respond);
//     return_status = check_phone_in_list(tmp.pNum, pBuff);

//     if(return_status == RETURN_OK)
//     {
//         printf("ADDRESS: %d\n", pBuff->addr);
//         printf("ADDRESS: %d\n", buff.addr);
//         // printf("AQI: %f\n", pBuff->aqi);
//         // printf("PM2.5: %f\n", pBuff->pm2_5);
//         // printf("CO: %f\n", pBuff->co);
//     }
//     return 0;
// }



#include "ec2x_def.h"
#include "serialPort.h"
#include "Lte.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <json-c/json.h>

#define PATH_TO_PHONE_NODE          "/tmp/phone_vs_node.json"
#define PATH_TO_DATA_NODE           "/tmp/aqi_and_concentration.json"

#define DEVICE "/dev/ttyUSB2"

typedef struct
{
    char pNum[100];
    char cmd[100];
} cmd_pNum_t;

typedef struct
{
    /* data */
    int addr;
    float aqi;
    float pm2_5;
    float co;
} data_of_node_t;

typedef enum {
    ADD_EXIST = 0,
    ADDR_NOT_EXIST,
    PHONE_NUM_EXIST,
    PHONE_NUM_NOT_EXIST
} return_status_t;

char respond[1024 * 1024];

/* Write command via USB port */
static Std_ReturnType SMS_SendWriteCommand(char *pcCommand, uint32_t u32CommandLen)
{
    Std_ReturnType ret = E_NOK;
    // char achResBuff[MAX_CMD_SIZE];
    // memset(achResBuff, 0, MAX_CMD_SIZE);

    char achResBuff[1024 * 1024];
    memset(achResBuff, 0, 1024 * 1024);

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
        strcpy(respond, achResBuff);
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

    usleep(100000);
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

cmd_pNum_t parse_msg(char *msg)
{
    cmd_pNum_t parsedVal;
    size_t len = strlen(msg);
    int i, count;
    int flag = 0;
    count = 0;

    if (msg == NULL)
    {
        return parsedVal;
    }

    for (i = 0; i < len; i++)
    {
        if (msg[i] == ',' && flag == 0)
        {
            memcpy(parsedVal.pNum, msg + i + 2, 12);
            parsedVal.pNum[12] = '\0';
            flag = 1;
        }
        if (msg[i] == ',')
        {
            count++;
        }

        if (count == 4)
        {
            memcpy(parsedVal.cmd, msg + i + 15, len - i);
            parsedVal.cmd[len] = '\0';
            break;
        }
    }
    return parsedVal;
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

   for(i = 0; i<len; i++)
   {
      tmp = json_object_array_get_idx(node, i);
      json_object_object_get_ex(tmp, "address", &addr);
      if(json_object_get_int(addr) == node_addr)
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

data_of_node_t check_phone_in_list(char *phoneNum)
{
    int lenPair, lenNum, i, j;
    data_of_node_t data;
    int checkFlag = 0;

    /* Get list of phone number and address of the node */
    json_object *root = json_object_from_file("/tmp/phone_vs_node.json");
    json_object *phone_and_node;
    json_object_object_get_ex(root, "phone_and_node_addr", &phone_and_node);

    lenPair = json_object_array_length(phone_and_node);

    json_object *tmp, *node_addr, *num_arr, *num, *phone_arr;

    for (i = 0; i < lenPair; i++)
    {
        tmp = json_object_array_get_idx(phone_and_node, i);
        /* Get phone number array of node i*/
        json_object_object_get_ex(tmp, "phone", &phone_arr);

        lenNum = json_object_array_length(phone_arr);
        for (j = 0; j < lenNum; j++)
        {
            /* Number j*/
            num_arr = json_object_array_get_idx(phone_arr, j);
            json_object_object_get_ex(num_arr, "num", &num);

            if(strcmp(json_object_get_string(num), phoneNum) == 0)
            {  
                checkFlag = 1;
                break;
            }
        }
        if(checkFlag)
        {
            /* Get node address */
            json_object_object_get_ex(tmp, "node_addr", &node_addr);
            data = parse_data(PATH_TO_DATA_NODE, json_object_get_int(node_addr));
            
            return data;
        }
    }
    data.addr = -1;
    return data;
}

void *read_msg(void *arg)
{
    int len;
    cmd_pNum_t cmd_phone;
    // read message in text mode
    send_to_uart("AT+CMGF=1");
    while (1)
    {
        send_to_uart("AT+CMG=\"ALL\"");
        if (strlen(respond) == 16)
        {
            goto time_break;
        }
        send_to_uart("AT+CMGR=0");
        cmd_phone = parse_msg(respond);
        // sleep 10s before handle
    time_break:
        sleep(10);
    }
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
            .u32BaudRate = 115200};

    ret = SerialPort_Open((char *)DEVICE);

    SerialPort_ConfigureTermios(&config);

    // find out which storages are supported 
    send_to_uart("AT+CMGF=1");

    send_to_uart("AT+CMGR=0");

    cmd_pNum_t tmp;
    data_of_node_t buff;
    tmp = parse_msg(respond);

    buff = check_phone_in_list(tmp.pNum);

    if(buff.addr == 0)
    {
        printf("Node not exist!\n");
    } 
    else if(buff.addr == -1)
    {
        printf("Phone number not exist!\n");
    }
    else 
    {
        printf("Data for node address: %d\n", buff.addr);
        printf("\t\tAQI:    %f\n", buff.aqi);
        printf("\t\tPM2.5:  %f\n", buff.pm2_5);
        printf("\t\tCO:    %f\n", buff.co);
    }
    return 0;
}

