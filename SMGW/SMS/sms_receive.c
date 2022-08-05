#include "ec2x_def.h"
#include "serialPort.h"
#include "Lte.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <json-c/json.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define PATH_TO_PHONE_NODE          "/tmp/phone_and_node.json"
#define PATH_TO_DATA_NODE           "/tmp/aqi_and_concentration.json"

#define HOUR_TO_SEND            9
#define MINUTE_TO_SEND          0



#define DEVICE "/dev/ttyUSB2"

sem_t mutex;

pthread_mutex_t lock;

typedef struct
{
    char pNum[100];
    char cmd[100];
} cmd_pNum_t;

typedef struct
{
    /* data */
    int addr;
    int aqi;
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

/* Send command to UART port*/
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

/* Send sms to phoneNumber with message msg */
void send_sms(char *msg, char *phoneNumber)
{
    char bufferPhone[30];
    sprintf(bufferPhone, "AT+CMGS=\"%s\"", phoneNumber);

    send_to_uart("AT");

    send_to_uart("AT+CMGF=1");
                
    send_to_uart(bufferPhone);
    send_to_uart(msg);
    send_to_uart("\x1A");
                
}

/* Retrive phone number and command from respone of message */
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
            len = strlen(parsedVal.cmd);
            parsedVal.cmd[len - 8] = '\0';

            break;
        }
    }
    return parsedVal;
}

/* Get AQI, CO... from json file, pair with node address */
data_of_node_t parse_data_from_json_file(char *PATH_TO_DATA, int node_addr)
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

/* Get full data by phone number of user */
data_of_node_t get_data_by_phone_num(char *phoneNum)
{
    int lenPair, lenNum, i, j;
    data_of_node_t data;
    int checkFlag = 0;

    /* Get list of phone number and address of the node */
    json_object *root = json_object_from_file(PATH_TO_PHONE_NODE);
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
            data = parse_data_from_json_file(PATH_TO_DATA_NODE, json_object_get_int(node_addr));
            
            return data;
        }
    }
    data.addr = -1;
    return data;
}

/* Respone to user by send sms */
int send_data_with_received_msg(char *respond)
{
    cmd_pNum_t cmd_and_pNum;
    data_of_node_t data_to_send;
    char msg[200];

    /* Parse sms to retrive phone numeber and command */
    cmd_and_pNum = parse_msg(respond);

    /* Check if command is "ALL" -> send all data to user */
    // if((strcmp(cmd_and_pNum.cmd, "ALL") == 0) && (strlen(cmd_and_pNum.cmd) == 3))
    // {
        data_to_send = get_data_by_phone_num(cmd_and_pNum.pNum);
        if((data_to_send.addr == 0)&& (strcmp(cmd_and_pNum.cmd, "ALL") == 0) && (strlen(cmd_and_pNum.cmd) == 3))
        {
            sprintf(msg, "Node khong ton tai!");
            send_sms(msg, cmd_and_pNum.pNum);
        } 
        else if((data_to_send.addr != -1)&& (strcmp(cmd_and_pNum.cmd, "ALL") != 0) && (strlen(cmd_and_pNum.cmd) != 3))
        {
            sprintf(msg, "Tin nhan sai cu phap. De xem chi tiet cac thong so hay gui ALL");
            send_sms(msg, cmd_and_pNum.pNum);
        }
        else if(data_to_send.addr == -1) 
        {
            sprintf(msg, "So dien thoai chua dang ky. De su dung dich vu, soan tin theo cua phap DK gui 6868");
            send_sms(msg, cmd_and_pNum.pNum);
        }
        else 
        {
            /* Attach parameter and send to user */
            sprintf(msg, "Chi tiet cac thong so nhu sau: AQI = %d, PM2.5 = %.2f, CO = %.2f\n", 
            data_to_send.aqi, data_to_send.pm2_5, data_to_send.co);
            send_sms(msg, cmd_and_pNum.pNum);
        }
    // }
}

/* Thread read message and send sms acording to command from user */
void *read_msg(void *arg)
{
    int len, i = 0;
    char cmd[20];
    cmd_pNum_t cmd_and_pNum;
    
    while (1)
    {
        sem_wait(&mutex);
        
        /* Set message in text mode */
        send_to_uart("AT+CMGF=1");

        /* Read all message */
        send_to_uart("AT+CMGL=\"ALL\"");
        if (strlen(respond) == 20)
        {
            printf("No message in SIM!\n");
            sem_post(&mutex);

            goto time_break;
        }        
        do 
        {

            /* Read message in i index */
            sprintf(cmd, "AT+CMGR=%d", i);
            send_to_uart(cmd);

            if(strlen(respond) != 16)
            {

                /* Send sms to user */
                send_data_with_received_msg(respond);

                /* Delete message in i index */
                sprintf(cmd, "AT+CMGD=%d", i);
                send_to_uart(cmd);
                i++;
            }
            else
            {

                /* Threre is no message at all */
                i = 0;
            }

        } while(i);
        sem_post(&mutex);

    // sleep 3s before handle another message
    time_break:
        sleep(3);

    }
}

/* Thread send daily sms */
void *send_daily(void *arg)
{
    int lenPair, lenNum, i, j;
    data_of_node_t data;
    char textToSend[300];
    int secSleep;
    time_t now, timeToSendSms;
    struct tm timeToSend;

    while(1)
    {
        // Get current time (now)
        now = time(NULL);
        
        // Copy current date to a `threepm`, and set time
        memcpy(&timeToSend, gmtime(&now), sizeof(struct tm));
        
        timeToSend.tm_hour = HOUR_TO_SEND;
        timeToSend.tm_min = MINUTE_TO_SEND;
        timeToSend.tm_sec = 0;

	    secSleep =(difftime(mktime(&timeToSend), now));

	    if(secSleep < 0)
	    {
	    	secSleep = 86400 + secSleep;
	    }

        sleep(secSleep);

        /* Get list of phone number and address of the node */
        json_object *root = json_object_from_file(PATH_TO_PHONE_NODE);
        json_object *phone_and_node;
        json_object_object_get_ex(root, "phone_and_node_addr", &phone_and_node);

        lenPair = json_object_array_length(phone_and_node);

        json_object *tmp, *node_addr, *num_arr, *num, *phone_arr;
        data_of_node_t parseDataFromAddr;

        for (i = 0; i < lenPair; i++)
        {
            tmp = json_object_array_get_idx(phone_and_node, i);

            /* Get node address */
            json_object_object_get_ex(tmp, "node_addr", &node_addr);
            parseDataFromAddr = parse_data_from_json_file(PATH_TO_DATA_NODE, json_object_get_int(node_addr));

            /* Compare value and send to user */ 
            if(parseDataFromAddr.aqi <= 50)
            {
                sprintf(textToSend, "Chi so AQI la: %d. Chat luong khong khi tot.", parseDataFromAddr.aqi);
            } 
            else if(parseDataFromAddr.aqi > 50 && parseDataFromAddr.aqi <= 100)
            {
                sprintf(textToSend, "Chi so AQI la: %d. Chat luong khong khi binh thuong.", parseDataFromAddr.aqi);
            }
            else if(parseDataFromAddr.aqi > 100 && parseDataFromAddr.aqi <= 150)
            {
                sprintf(textToSend, "Chi so AQI la: %d. Chat luong khong khi kem.", parseDataFromAddr.aqi);
            }
            else if(parseDataFromAddr.aqi > 150 && parseDataFromAddr.aqi <= 200)
            {
                sprintf(textToSend, "Chi so AQI la: %d. Chat luong khong khi xau. ", parseDataFromAddr.aqi);
            }
            else if(parseDataFromAddr.aqi > 200 && parseDataFromAddr.aqi <= 300)
            {
                sprintf(textToSend, "Chi so AQI la: %d. Chat luong khong khi ra xau.", parseDataFromAddr.aqi);
            }
            else if(parseDataFromAddr.aqi > 300 && parseDataFromAddr.aqi <= 500)
            {
                sprintf(textToSend, "Chi so AQI la: %d. Chat luong khong khi nguy hai.", parseDataFromAddr.aqi);
            }
            

            /* Get phone number array of node i*/
            json_object_object_get_ex(tmp, "phone", &phone_arr);

            lenNum = json_object_array_length(phone_arr);
            for (j = 0; j < lenNum; j++)
            {
                /* Number j*/
                sem_wait(&mutex);
                num_arr = json_object_array_get_idx(phone_arr, j);
                json_object_object_get_ex(num_arr, "num", &num);
                send_sms(textToSend, json_object_get_string(num));
                sem_post(&mutex);
                sleep(1);

            }
            
        }   
    }
}

/* Thread update file phone_vs_node.*/
void *update_file_phone_vs_node(void *arg)
{

}

int main()
{
    sem_init(&mutex, 0, 1);

    pthread_t thread_response_usr_id;
    pthread_t thread_send_daily;

    Std_ReturnType ret = 0;
    SerialPort_TermiosConfigType config =
    {
        .bParityBitEnable = false,
        .bStopBitEnable = true,
        .bFlowControlEnable = false,
        .u32BaudRate = 115200};

    ret = SerialPort_Open((char *)DEVICE);

    /* Setting UART port */
    SerialPort_ConfigureTermios(&config);
    pthread_create(&thread_response_usr_id, NULL, read_msg, NULL);
    pthread_create(&thread_send_daily, NULL, send_daily, NULL);
       
    pthread_join(thread_response_usr_id, NULL);
    pthread_join(thread_send_daily, NULL);

    return 0;
}