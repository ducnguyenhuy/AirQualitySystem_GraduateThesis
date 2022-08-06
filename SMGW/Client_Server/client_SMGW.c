#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <limits.h>


#define SERVER_IP           "202.191.56.104"
// #define SERVER_IP           "192.168.168.108"
#define PORT_IP             5592
#define PORT_AQI_CON        5591
#define PORT_ERROR_NODE     5590

#define SEND_FIRST_TIME     0
#define SEND_N_TIME         1

#define SEND_OK             0
#define SEND_FAILED         1

#define FILE_PHONE_VS_NODE          "/tmp/phone_and_node.json"
#define FILE_AQI_CON                "/tmp/aqi_and_concentration.json"
#define FILE_NODE_NOT_WORK          "/tmp/node_not_work.txt"

#define SIZE 1024

#define HOUR_TO_SEND    9
#define MINUTE_TO_SEND  0

#define BUF_LEN          (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int gSendIpStatus = SEND_FAILED;

char *gIpAddr = NULL;

char *get_ip(void)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char *host = (char *)malloc(NI_MAXHOST * sizeof(char));

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if ((strcmp(ifa->ifa_name, "eth0") == 0) && (ifa->ifa_addr->sa_family == AF_INET))
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            if (host[0] == '1')
            {
                freeifaddrs(ifaddr);
                return host;
            }
        }
        else if ((strcmp(ifa->ifa_name, "eth1") == 0) && (ifa->ifa_addr->sa_family == AF_INET))
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            if (host[0] == '1')
            {
                freeifaddrs(ifaddr);
                return host;
            }
        }
    }
}

/* Send ip address to server */
void *sock_send_ip(void *arg)
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    int flag_send_ip;
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
            // Similar and not is first time
            if ((flag_send_ip != SEND_FIRST_TIME) && (gSendIpStatus == SEND_OK))
            {
                sleep(5);
            }
            else
            {
                gSendIpStatus = SEND_FAILED;

                // Send ip address if it is not first time and has different
                sprintf(cmd, "%s", gIpAddr);
                write(clientSocket, cmd, strlen(cmd));

                clientSocket = socket(PF_INET, SOCK_STREAM, 0);
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_port = htons(PORT_IP);
                serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
                memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
                addr_size = sizeof(serverAddr);

                sleep(1);
            }

            flag_send_ip = SEND_N_TIME;
        }
    }
}

int write_file(int sockfd)
{
    int n;
    FILE *fp;
    char buffer[SIZE];

    fp = fopen(FILE_PHONE_VS_NODE, "w");

    if (fp == NULL)
    {
        perror("Error in creating file.");
        exit(1);
    }

    while (1)
    {
        n = recv(sockfd, buffer, SIZE, 0);
        if (n <= 0)
        {
            printf("Start updating %s ...\n", FILE_PHONE_VS_NODE);
            fclose(fp);
            printf("Update successfully!\n", FILE_PHONE_VS_NODE);
            break;
            return 0;
        }
        if (!strcmp(buffer, gIpAddr))
        {
            printf("Received ACK: %s\n", buffer);
            gSendIpStatus = SEND_OK;
            return 0;
        }
        
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    }
}

/* Receive list user */
void *sock_receive_link(void *arg)
{
    int e;
    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7001);
    server_addr.sin_addr.s_addr = inet_addr(gIpAddr);

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd, 10);

    printf("Listening on socket receive list user...\n");

    while (1)
    {
        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd, (struct sockaddr *)&new_addr, &addr_size);
        gSendIpStatus = SEND_OK;

        write_file(new_sock);
    }
}

void send_file(FILE *fp, int sockfd)
{
    char data[SIZE] = {0};

    while (fgets(data, SIZE, fp) != NULL)
    {
        if (write(sockfd, data, strlen(data)) == -1)
        {
            perror("Error in sendung data");
            exit(1);
        }

        usleep(100000);
        bzero(data, SIZE);
    }
}

/* Send ip address to server */
void *sock_send_file_AQI(void *arg)
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    FILE *fp = NULL;
    char cmd[50];

    int secSleep;
    time_t now, timeToSendSms;
    struct tm timeToSend;

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_AQI_CON);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof(serverAddr);

    while (1)
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
        printf("Sleep %d seconds befor send file %s!\n", secSleep, FILE_AQI_CON);
        sleep(secSleep);

        if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
        {
            fp = fopen(FILE_AQI_CON, "r");
            if (fp == NULL)
            {
                perror("[-]Error in reading file.");
                exit(1);
            }

            send_file(fp, clientSocket);
            printf("File %s send successfully \n", FILE_AQI_CON);
            close(clientSocket);

            clientSocket = socket(PF_INET, SOCK_STREAM, 0);
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(PORT_AQI_CON);
            serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
            memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
            addr_size = sizeof(serverAddr);
        }
        
    }
}


/* Send ip address to server */
void *sock_send_node_error(void *arg)
{
    char cmd[50];
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_ERROR_NODE);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof(serverAddr);

    while (1)
    {
        int inotifyFd, wd;
        char buff[BUF_LEN];
        uint32_t numRead;
        char *p;
        FILE *fp  = NULL;
        struct inotify_event *event;

        /* Create inotify instance */
        inotifyFd = inotify_init();

        wd = inotify_add_watch(inotifyFd, FILE_NODE_NOT_WORK, IN_CLOSE_WRITE);

        printf("Start watching %s\n", FILE_NODE_NOT_WORK);
        for (;;)
        { /* Read events forever */
            numRead = read(inotifyFd, buff, BUF_LEN);

            event = (struct inotify_event *)buff;
            if (event->mask & IN_CLOSE_WRITE)
            {

                if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
                {
                        fp = fopen(FILE_NODE_NOT_WORK, "r");
                        if (fp == NULL)
                        {
                            perror("Error in reading file.");
                            exit(1);
                        }

                        send_file(fp, clientSocket);
                        printf("File %s send successfully!\n", FILE_NODE_NOT_WORK);
                        close(clientSocket);

                        clientSocket = socket(PF_INET, SOCK_STREAM, 0);
                        serverAddr.sin_family = AF_INET;
                        serverAddr.sin_port = htons(PORT_ERROR_NODE);
                        serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
                        memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
                        addr_size = sizeof(serverAddr);
                }
            }
        }
    }
}

int main()
{
    printf("Start running client on SMGW!\n");

    pthread_t thread_send_ip;
    pthread_t thread_receive_ip;
    pthread_t thread_send_AQI;
    pthread_t thread_send_NODE_ERROR;

    // get ip address
    gIpAddr = get_ip();

    pthread_create(&thread_send_ip, NULL, sock_send_ip, NULL);
    pthread_create(&thread_receive_ip, NULL, sock_receive_link, NULL);
    pthread_create(&thread_send_AQI, NULL, sock_send_file_AQI, NULL);
    pthread_create(&thread_send_NODE_ERROR, NULL, sock_send_node_error, NULL);

    pthread_join(thread_send_ip, NULL);
    pthread_join(thread_send_AQI, NULL);
    pthread_join(thread_receive_ip, NULL);
    pthread_join(thread_send_NODE_ERROR, NULL);

    return 0;
}
