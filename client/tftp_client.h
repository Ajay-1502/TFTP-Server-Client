#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H

#include "tftp.h"
#include <netinet/in.h>

typedef struct
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len;
    char server_ip[INET_ADDRSTRLEN];
    int mode;
    int connected;
} tftp_client_t;

void process_command(tftp_client_t *client, char *command);
void connect_to_server(tftp_client_t *client);
void put_file_client(tftp_client_t *client);
void get_file_client(tftp_client_t *client);
void disconnect_client(tftp_client_t *client);

#endif