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


#define FLIE_LINK               "./file_store_link.txt"
#define FILE_TO_SEND            "./phone_vs_node.json"
#define PATH_TO_FILE_AQI_CON    "./AQI_and_Concentration.json"

#define BUF_LEN             (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

#define PORT_IP             5592
#define PORT_AQI_CON        5591
#define PORT_ERROR_NODE     5590

// #define SERVER_IP           "192.168.168.108"
#define SERVER_IP           "202.191.56.104"

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
        printf("data: %s\n", data);
        if(send(sockfd, data, sizeof(data), 0)== -1)
        {
            perror("[-] Error in sendung data");
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
        sleep(1);
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
                printf("IN_CLOSE_WRITE!\n");
                if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
                {
                    printf("[+]Connected to server.\n");
                    fp = fopen(FILE_TO_SEND, "r");
                    if(fp == NULL)
                    {
                        perror("[-]Error in reading file.");
                        exit(1);
                    }
                    send_file(fp,clientSocket);
                    printf("[+] File data send successfully. \n");
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

int write_file(int sockfd)
{
    int n; 
    FILE *fp;
    char buffer[SIZE];

    fp = fopen(PATH_TO_FILE_AQI_CON, "w");
    if(fp==NULL)
    {
        perror("[-]Error in creating file.");
        exit(1);
    }

    while(1)
    {
        n = recv(sockfd, buffer, SIZE, 0);
        if(n<=0)
        {
            fclose(fp);
            break;
            return 0;
        }
        
        fprintf(fp, "%s", buffer);
        printf("Update to file %s!\n", PATH_TO_FILE_AQI_CON);
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
    
    printf("[+]Listening...\n");
    
    while (1)
    {
        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd,(struct sockaddr*)&new_addr, &addr_size);

        if(!write_file(new_sock))
        {
            printf("Update done!\n");
        }
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
    
    printf("[+]Listening...\n");
    
    while (1)
    {
        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd,(struct sockaddr*)&new_addr, &addr_size);

        if(!write_file(new_sock))
        {
            printf("Update done!\n");
        }
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