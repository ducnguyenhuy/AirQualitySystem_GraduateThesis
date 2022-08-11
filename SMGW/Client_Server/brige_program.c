#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <limits.h>
#include <sys/wait.h>
#include <time.h>

// #define SERVER_PORT_IP              1
// #define SERVER_PORT_AQI_CON         2
// #define SERVER_PORT_ERROR_NODE      3
// #define SERVER_PORT_USER_AND_ACK    4

// #define CLIENT_PORT_IP              5
// #define CLIENT_PORT_AQI_CON         6
// #define CLIENT_PORT_ERROR_NODE      7
// #define CLIENT_PORT_USER_AND_ACK    8

// #define SERVER_IP           "202.191.56.104"
// #define LAPTOP_IP           "192.168.168.108"

/* Send ip address to server */
int send_IP(char *ip_smgw)
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    char cmd[100];

    // Config socket
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_IP);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof serverAddr;

    while (1)
    {
        if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
        {
            // Send ip address if it is not first time and has different
            sprintf(cmd, "%s", ip_smgw);
            write(clientSocket, cmd, strlen(cmd));
            return 1;
        }

    }
}

/* Receive IP from SMGW and send to Server*/
void *thread_receive_IP(void *arg)
{
    int serverSocket = -1;
    int newSocket = -1;
    struct sockaddr_in server_addr;

    int readValue = 1;
    uint8_t *buffer = (uint8_t *)malloc(1024 * sizeof(uint8_t));

    memset(buffer, 0, sizeof(buffer));
    memset(&server_addr, 0, sizeof(server_addr));

    //Config socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT_IP);
    
    bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(serverSocket, 10);

    while(1)
    {
        newSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
        readValue = read(newSocket, buffer, 1024);

        if(readValue)
        {
            sock_send_ip(buffer);
        }
    }
}

/* Send ip address to server */
int send_ACK(char *ip_smgw)
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    char cmd[100];

    // Config socket
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_IP);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof serverAddr;

    while (1)
    {
        if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
        {
            // Send ip address if it is not first time and has different
            sprintf(cmd, "%s", ip_smgw);
            write(clientSocket, cmd, strlen(cmd));
            return 1;
        }

    }
}

/* Receive IP from SMGW and send to Server*/
void *thread_receive_ACK(void *arg)
{
    int serverSocket = -1;
    int newSocket = -1;
    struct sockaddr_in server_addr;

    int readValue = 1;
    uint8_t *buffer = (uint8_t *)malloc(1024 * sizeof(uint8_t));

    memset(buffer, 0, sizeof(buffer));
    memset(&server_addr, 0, sizeof(server_addr));

    //Config socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT_IP);
    
    bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(serverSocket, 10);

    while(1)
    {
        newSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
        readValue = read(newSocket, buffer, 1024);

        if(readValue)
        {
            sock_send_ip(buffer);
        }
    }
}

/* Send ip address to server */
int send_NODE_NOT_WORK(char *ip_smgw)
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    char cmd[100];

    // Config socket
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_IP);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof serverAddr;

    while (1)
    {
        if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
        {
            // Send ip address if it is not first time and has different
            sprintf(cmd, "%s", ip_smgw);
            write(clientSocket, cmd, strlen(cmd));
            return 1;
        }

    }
}

/* Receive IP from SMGW and send to Server*/
void *thread_receive_NODE_NOT_WORK(void *arg)
{
    int serverSocket = -1;
    int newSocket = -1;
    struct sockaddr_in server_addr;

    int readValue = 1;
    uint8_t *buffer = (uint8_t *)malloc(1024 * sizeof(uint8_t));

    memset(buffer, 0, sizeof(buffer));
    memset(&server_addr, 0, sizeof(server_addr));

    //Config socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT_IP);
    
    bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(serverSocket, 10);

    while(1)
    {
        newSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
        readValue = read(newSocket, buffer, 1024);

        if(readValue)
        {
            sock_send_ip(buffer);
        }
    }
}

/* Send ip address to server */
int send_FILE_AQI(char *ip_smgw)
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    char cmd[100];

    // Config socket
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_IP);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof serverAddr;

    while (1)
    {
        if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
        {
            // Send ip address if it is not first time and has different
            sprintf(cmd, "%s", ip_smgw);
            write(clientSocket, cmd, strlen(cmd));
            return 1;
        }

    }
}

/* Receive IP from SMGW and send to Server*/
void *thread_receive_FILE_AQI(void *arg)
{
    int serverSocket = -1;
    int newSocket = -1;
    struct sockaddr_in server_addr;

    int readValue = 1;
    uint8_t *buffer = (uint8_t *)malloc(1024 * sizeof(uint8_t));

    memset(buffer, 0, sizeof(buffer));
    memset(&server_addr, 0, sizeof(server_addr));

    //Config socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT_IP);
    
    bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(serverSocket, 10);

    while(1)
    {
        newSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
        readValue = read(newSocket, buffer, 1024);

        if(readValue)
        {
            sock_send_ip(buffer);
        }
    }
}

int main()
{
    
}






