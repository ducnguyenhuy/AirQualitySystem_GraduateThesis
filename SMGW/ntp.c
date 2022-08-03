#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Difference between Jan 1, 1900 and Jan 1, 1970
#define UNIX_OFFSET 2208988800L
#define NTP_DEFAULT_PORT "123"
#define NTP_FLAGS 0x23

typedef struct
{

    uint8_t flags;

    uint8_t stratum;
    uint8_t poll;

    uint8_t precision;

    uint32_t root_delay;
    uint32_t root_dispersion;

    uint8_t referenceID[4];

    uint32_t ref_ts_secs;
    uint32_t ref_ts_frac;

    uint32_t origin_ts_secs;
    uint32_t origin_ts_frac;

    uint32_t recv_ts_secs; // This is what we need mostly to get current time.
    uint32_t recv_ts_fracs;

    uint32_t transmit_ts_secs;
    uint32_t transmit_ts_frac;
} ntp_packet;

typedef struct ntpc
{
    char server[64];
    int timezone;
} ntpConfig_t;

ntpConfig_t g_config;

void update_time(void)
{
    char cmd[30];
    char *port = NTP_DEFAULT_PORT;
    int ret = -1;

    struct hostent *svr = NULL;
    int server_sock, status;
    struct addrinfo hints, *servinfo, *ap;
    socklen_t addrlen = sizeof(struct sockaddr_storage);

    printf("Start receive hostname!\n");
    // check if internet accessed
    svr = gethostbyname("vn.pool.ntp.org");

    while (svr == NULL)
    {
        printf("Retry!\n");
        sleep(5);
        svr = gethostbyname(g_config.server);
    }

    ntp_packet packet = {.flags = NTP_FLAGS}; // populate the struct

    hints = (struct addrinfo){.ai_family = AF_UNSPEC, .ai_socktype = SOCK_DGRAM};

    // Get the info of the NTP server
    if ((status = getaddrinfo(g_config.server, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
        exit(2);
    }

    // Initalize the socket
    for (ap = servinfo; ap != NULL; ap = ap->ai_next)
    {
        server_sock = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
        if (server_sock == -1)
            continue;
        break;
    }

    if (ap == NULL)
    {
        fprintf(stderr, "socket() error\n");
        exit(2);
    }

    // Send the structure to the server
    if ((status = sendto(server_sock, &packet, sizeof(packet), 0, ap->ai_addr, addrlen)) == -1)
    {
        perror("sendto() error");
        exit(2);
    }
    // Send the structure to the server
    if ((status = recvfrom(server_sock, &packet, sizeof(packet), 0, ap->ai_addr, &addrlen)) == -1)
    {
        perror("recvfrom() error");
        exit(2);
    }
    freeaddrinfo(servinfo);
    // pclose(server_sock);

    // Convert from network's to host's endian order
    packet.recv_ts_secs = ntohl(packet.recv_ts_secs);

    // The number of seconds we get back from the server is equal to the no. of
    // seconds since Jan 1, 1900. Since Unix time starts from Jan 1, 1970, we
    // subtract 70 years worth of seconds from Jan 1, 1990.
    time_t time_struct = (time_t)packet.recv_ts_secs - UNIX_OFFSET;
    char buf[80];
    struct tm ts;
    ts = *localtime(&(time_struct));
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);

    printf("%s\n", buf);
    sprintf(cmd, "date -s \"%s\"", buf);

    printf("Cmd: %s\n", cmd);
    system(cmd);
}

int main(void)
{
    strcpy(g_config.server, "vn.pool.ntp.org");
    g_config.timezone = 7;

    update_time();

    return 0;
}