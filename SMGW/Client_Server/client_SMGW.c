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

#define SERVER_IP       "192.168.168.108"
#define SERVER_PORT     7000

#define SEND_FIRST_TIME 0
#define SEND_N_TIME     1

#define SEND_OK         0
#define SEND_FAILED     1

#define PATH_TO_FILE_PHONE_VS_NODE  "./phone_vs_node.json"

#define SIZE 1024

int gSendIpStatus = SEND_FAILED;

char *gIpAddr = NULL;
int gPort = 7000;

char gLink[100];

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

        s = getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name,"eth0")==0) && (ifa->ifa_addr->sa_family==AF_INET))
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            if(host[0] == '1')
            {
                freeifaddrs(ifaddr);
                return host;
            }
        } else if((strcmp(ifa->ifa_name,"eth1")==0) && (ifa->ifa_addr->sa_family==AF_INET))
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            if(host[0] == '1')
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
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof serverAddr;

    while(1)
    {
        if (!connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size))
        {
            // Similar and not is first time
            if((flag_send_ip != SEND_FIRST_TIME) && (gSendIpStatus == SEND_OK))
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
                serverAddr.sin_port = htons(SERVER_PORT);
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

    fp = fopen(PATH_TO_FILE_PHONE_VS_NODE, "w");
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
        if(!strcmp(buffer, gIpAddr))
        {
            printf("Received ACK: %s\n", buffer);
            gSendIpStatus = SEND_OK;
            fclose(fp);
            return 0;
        }

        fprintf(fp, "%s", buffer);
        printf("Update to file %s!\n", PATH_TO_FILE_PHONE_VS_NODE);
        bzero(buffer, SIZE);
    }
}

/* Receive link to wget new file json */
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

    bind(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(sockfd, 10);
    
    printf("[+]Listening...\n");
    
    while (1)
    {
        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd,(struct sockaddr*)&new_addr, &addr_size);
        gSendIpStatus = SEND_OK;

        if(!write_file(new_sock))
        {
            printf("Update done!\n");
        }
    }
}

int main()
{
    printf("Start running client on SMGW!\n");

    pthread_t thread_send_ip;
    pthread_t thread_receive_ip;

    // get ip address
    gIpAddr = get_ip();

    pthread_create(&thread_send_ip, NULL, sock_send_ip, NULL);
    pthread_create(&thread_receive_ip, NULL, sock_receive_link, NULL);

    pthread_join(thread_send_ip, NULL);
    pthread_join(thread_receive_ip, NULL);

    return 0;
}
