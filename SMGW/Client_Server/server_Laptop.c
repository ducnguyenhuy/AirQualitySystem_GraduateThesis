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

#define FLIE_LINK   "./file_store_link.txt"
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

#define PORT_NUM            7000
#define SERVER_IP           "192.168.168.108"

#define RECEIVE_IP          0
#define NOT_RECEIVE_IP      1

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
    server_addr.sin_port = htons(PORT_NUM);
    
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

/* Send ip address to server */
void *sock_send_link(void *arg)
{
    char cmd[100];
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
        int inotifyFd, wd, j;
        char buf[BUF_LEN];
        ssize_t numRead;
        char *p;
        struct inotify_event *event;
        FILE *f_link;

        /* Create inotify instance */
        inotifyFd = inotify_init();
        if (inotifyFd == -1)
        {
            printf("inotify_init!\n");
        }
        
        wd = inotify_add_watch(inotifyFd, FLIE_LINK, IN_CLOSE_WRITE);
        if (wd == -1)
        {
            printf("inotify_add_watch error!\n");
        }
        printf("Start watching %s using\n", FLIE_LINK);

        f_link = fopen(FLIE_LINK, "r");

        for (;;)
        { /* Read events forever */
            numRead = read(inotifyFd, buf, BUF_LEN);
            if (numRead == 0)
                printf("read() from inotify fd returned 0!");
            if (numRead == -1)
                printf("read");
            if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
            {
                char IPbuffer[100];
                fgets(IPbuffer, 100, f_link);

                sprintf(cmd, "%s", IPbuffer);
                write(clientSocket, cmd, strlen(cmd));
                printf("Sent: %s\n", cmd);
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


int main()
{
    pthread_t thread_receive_ip;
    pthread_t thread_send_ip;

    printf("Start running server on Laptop!\n");

    pthread_create(&thread_receive_ip, NULL, sock_receive_ip, NULL);
    pthread_create(&thread_send_ip, NULL, sock_send_link, NULL);


    pthread_join(thread_receive_ip, NULL);
    pthread_join(thread_send_ip, NULL);

	return 0;

}