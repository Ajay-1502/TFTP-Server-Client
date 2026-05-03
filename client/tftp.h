
#ifndef TFTP_H
#define TFTP_H

#include <stdint.h>
#include <arpa/inet.h>

#define PORT 3020
#define BUFFER_SIZE 516
#define TIMEOUT_SEC 5
#define MAX_RETRIES 3

// Modes
#define MODE_NORMAL 1
#define MODE_OCTET 2
#define MODE_NETASCII 3

// TFTP OpCodes
typedef enum
{
    RRQ = 1,
    WRQ = 2,
    DATA = 3,
    ACK = 4,
    ERROR = 5
} tftp_opcode;

void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename, int mode);
void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename, int mode);

#endif