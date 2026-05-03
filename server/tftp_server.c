/*
 Project: TFTP Client-Server File Transfer System
 Author: Ajay Desai
 Roll_No : 25031A_105
 Description: TFTP-based client-server application in C using UDP sockets to enable reliable file transfer with block-based communication and acknowledgment mechanism.
*/

#include "tftp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *buffer);

int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Set timeout
    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    printf("TFTP Server listening on port %d...\n", PORT);

    while (1)
    {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&client_addr, &client_len);

        if (n < 0)
        {
            // perror("Receive failed or timeout");
            continue;
        }

        handle_client(sockfd, client_addr, client_len, buffer);
    }

    close(sockfd);
    return 0;
}

void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *buffer)
{
    uint16_t opcode = ntohs(*(uint16_t *)buffer);
    char filename[256];
    strcpy(filename, buffer + 2);

    printf("Request received for file: %s\n", filename);

    if (opcode == RRQ)
    {
        printf("RRQ request\n");

        FILE *fp = fopen(filename, "rb");
        if (!fp)
        {
            printf("File not found\n");

            char err[BUFFER_SIZE];
            *(uint16_t *)err = htons(ERROR);
            *(uint16_t *)(err + 2) = htons(1);
            strcpy(err + 4, "File not found");

            sendto(sockfd, err, strlen(err + 4) + 4, 0,
                   (struct sockaddr *)&client_addr, client_len);
            return;
        }

        fclose(fp);
        send_file(sockfd, client_addr, client_len, filename, MODE_NORMAL);
    }
    else if (opcode == WRQ)
    {
        printf("WRQ request\n");

        FILE *fp = fopen(filename, "wb"); // create or truncate
        if (!fp)
        {
            printf("File create failed\n");
            return;
        }
        fclose(fp);

        // Send ACK block 0
        char ack[4];
        *(uint16_t *)ack = htons(ACK);
        *(uint16_t *)(ack + 2) = htons(0);

        sendto(sockfd, ack, 4, 0,
               (struct sockaddr *)&client_addr, client_len);

        receive_file(sockfd, client_addr, client_len, filename, MODE_NORMAL);
    }
    else
    {
        printf("Unknown opcode\n");
    }
}