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

// #define SERVER_IP           "192.168.168.108"
#define SERVER_IP           "202.191.56.104"

#define PORT_IP             5592
#define PORT_AQI_CON        5591
#define PORT_ERROR_NODE     5590

#define FLIE_LINK                   "./file_store_link.txt"
#define FILE_USER                   "./phone_and_node.json"
#define FILE_AQI_CON                "./aqi_and_concentration.json"
#define FILE_NODE_NOT_WORK          "./node_not_work.txt"

#define BUF_LEN             (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))


#define RECEIVE_IP          0
#define NOT_RECEIVE_IP      1

#define SIZE 1024

char *ipReceived = NULL;
int flag_receive_ip = NOT_RECEIVE_IP;

void *sock_receive_ip(void *arg)
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
            ipReceived = buffer;
            printf("ipReceived: %s\n", ipReceived);
            flag_receive_ip = RECEIVE_IP;
        }

        if(flag_receive_ip == RECEIVE_IP)
        {
            char cmd[100];
            int clientSocket;
            struct sockaddr_in serverAddr;
            socklen_t addr_size;

            clientSocket = socket(PF_INET, SOCK_STREAM, 0);
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(7001);
            serverAddr.sin_addr.s_addr = inet_addr(ipReceived);
            memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
            addr_size = sizeof(serverAddr);

            if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
            {
                printf("Received IP, send back ACK!\n");
                sprintf(cmd, "%s", ipReceived);
                write(clientSocket, cmd, strlen(cmd));
                flag_receive_ip = NOT_RECEIVE_IP;

                clientSocket = socket(PF_INET, SOCK_STREAM, 0);
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_port = htons(7001);
                serverAddr.sin_addr.s_addr = inet_addr(ipReceived);
                memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
                addr_size = sizeof(serverAddr);
            }
    }
    }
}

void send_file(FILE *fp, int sockfd)
{
    char data[SIZE] = {0};

    while(fgets(data, SIZE, fp)!=NULL)
    {
        if(send(sockfd, data, sizeof(data), 0)== -1)
        {
            perror("Error in sendung data");
            exit(1);
        }
        usleep(100000);
        bzero(data, SIZE);
    }
}

/* Send ip address to server */
void *sock_send_file_user(void *arg)
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    while(ipReceived == NULL)
    {
        printf("Wait to receive IP!\n");
        sleep(2);
    }
    
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7001);
    serverAddr.sin_addr.s_addr = inet_addr(ipReceived);

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof(serverAddr);

    while(1)
    {
        int inotifyFd, wd;
        char buff[BUF_LEN];
        uint32_t numRead;
        char *p;
        FILE *fp  = NULL;
        struct inotify_event *event;

        /* Create inotify instance */
        inotifyFd = inotify_init();

        wd = inotify_add_watch(inotifyFd, FLIE_LINK, IN_CLOSE_WRITE);

        printf("Start watching %s\n", FLIE_LINK);
        for (;;)
        { /* Read events forever */
            numRead = read(inotifyFd, buff, BUF_LEN);

            event = (struct inotify_event *)buff;
            if (event->mask & IN_CLOSE_WRITE)
            {
                if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
                {
                    printf("Connected to server\n");
                    fp = fopen(FILE_USER, "r");
                    if(fp == NULL)
                    {
                        perror("Error in reading file.");
                        exit(1);
                    }
                    printf("Start sending %s ...\n", FILE_USER);
                    send_file(fp,clientSocket);
                    printf("Send %s successfully!\n", FILE_USER);
                    close(clientSocket);   

                    sleep(5);     
                    
                    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
                    serverAddr.sin_family = AF_INET;
                    serverAddr.sin_port = htons(7001);
                    serverAddr.sin_addr.s_addr = inet_addr(ipReceived);
                    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
                    addr_size = sizeof(serverAddr); 
                } 
        }
    }
}
}

int write_file(int sockfd, char *file_name)
{
    int n; 
    FILE *fp;
    char buffer[SIZE];
    char file_aqi[100];
    // write to file AQI with timestamp
    if(!strcmp(file_name, FILE_AQI_CON))
    {
        time_t rawtime;
        struct tm * timeinfo;
        char output[100];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        sprintf(file_aqi, "./aqi_and_concentration_%d%d%d_%d%d%d.json", 
        timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        file_name = file_aqi;
    }

    fp = fopen(file_name, "w");
    if(fp==NULL)
    {
        perror("Error in creating file.");
        exit(1);
    }
    printf("Start updating %s ...\n", file_name);

    while(1)
    {
        n = recv(sockfd, buffer, SIZE, 0);
        if(n<=0)
        {
            fclose(fp);
            printf("Update successfully!\n");
            break;
            return 0;
        }
        
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    }
}

/* Receive link to wget new file json */
void *sock_receive_file_AQI(void *arg)
{
    int e;
    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_AQI_CON);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    bind(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(sockfd, 10);
    
    printf("Listening on socket receive file AQI\n");
    
    while (1)
    {
        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd,(struct sockaddr*)&new_addr, &addr_size);

        write_file(new_sock, FILE_AQI_CON);
    }
}

void *sock_receive_node_error(void *arg)
{
   int e;
    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_ERROR_NODE);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    bind(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(sockfd, 10);
    
    printf("Listening on socket receive file node not working\n");
    
    while (1)
    {
        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd,(struct sockaddr*)&new_addr, &addr_size);

        write_file(new_sock, FILE_NODE_NOT_WORK);
    }
}


int main()
{
    pthread_t thread_receive_ip;
    pthread_t thread_send_ip;
    pthread_t thread_receive_AQI;
    pthread_t thread_receive_node_error;

    printf("Start running server on Laptop!\n");



    pthread_create(&thread_receive_ip, NULL, sock_receive_ip, NULL);
    pthread_create(&thread_send_ip, NULL, sock_send_file_user, NULL);
    pthread_create(&thread_receive_AQI, NULL, sock_receive_file_AQI, NULL);
    pthread_create(&thread_receive_node_error, NULL, sock_receive_node_error, NULL);


    pthread_join(thread_receive_ip, NULL);
    pthread_join(thread_send_ip, NULL);
    pthread_join(thread_receive_AQI, NULL);
    pthread_join(thread_receive_node_error, NULL);

	return 0;

}