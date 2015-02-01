#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

class UdpSender{
    int sock;
    struct sockaddr_in echoserver;

public:
    UdpSender() {
        if (init("10.1.1.2", 65535) != 0) {
            printf("socket init failed\n");
            exit(1);
        }
    }

    int init(const char* ipStr, int port) {
        /* Create the UDP socket */
        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            printf("initialize socket Failed\n");
            return -1;
        }
        /* Construct the server sockaddr_in structure */
        memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
        echoserver.sin_family = AF_INET;                  /* Internet/IP */
        echoserver.sin_addr.s_addr = inet_addr(ipStr);  /* IP address */
        echoserver.sin_port = htons(port);       /* server port */
        return 0;
    }

    int sendMessage(char* buffer, int len) {
        /* Send the word to the server */
        if (sendto(sock, buffer, len, 0,
                   (struct sockaddr *) &echoserver,
                   sizeof(echoserver)) != len) {
            return -1;
        }
        return 0;
    }
} udpSender;

/*
#define BUFFSIZE 255

void Die(const char *mess) { perror(mess); exit(1); }

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer[BUFFSIZE];
    unsigned int echolen, clientlen;
    int received = 0;

    if (argc != 4) {
      fprintf(stderr, "USAGE: %s <server_ip> <word> <port>\n", argv[0]);
      exit(1);
    }

    // Create the UDP socket /
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      Die("Failed to create socket");
    }
    // Construct the server sockaddr_in structure /
    memset(&echoserver, 0, sizeof(echoserver));       // Clear struct //
    echoserver.sin_family = AF_INET;                  // Internet/IP //
    echoserver.sin_addr.s_addr = inet_addr(argv[1]);  // IP address //
    echoserver.sin_port = htons(atoi(argv[3]));       // server port //

    / Send the word to the server /
    echolen = strlen(argv[2]);
    if (sendto(sock, argv[2], echolen, 0,
               (struct sockaddr *) &echoserver,
               sizeof(echoserver)) != echolen) {
      Die("Mismatch in number of sent bytes");
    }
    if (sendto(sock, argv[2], echolen, 0,
               (struct sockaddr *) &echoserver,
               sizeof(echoserver)) != echolen) {
      Die("Mismatch in number of sent bytes");
    }
}
*/
