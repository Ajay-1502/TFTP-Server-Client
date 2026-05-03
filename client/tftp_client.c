
/*
 Project: TFTP Client-Server File Transfer System
 Author: Ajay Desai
 Roll_No : 25031A_105
 Description: TFTP-based client-server application in C using UDP sockets to enable reliable file transfer with block-based communication and acknowledgment mechanism.
*/

#include "tftp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int validate_ip(char *ip)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr));
}

int main()
{
    tftp_client_t client;
    memset(&client, 0, sizeof(client));
    client.mode = MODE_NORMAL;

    char command[256];

    while (1)
    {
        printf("==============================================\n");
        printf("              T F T P   M E N U             \n");
        printf("==============================================\n\n");
        printf("1. connect\n");
        printf("2. put\n");
        printf("3. get\n");
        printf("4. mode\n");
        printf("5. exit\n\n");
        printf("TYPE THE OPTION : ");

        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        process_command(&client, command);
    }

    return 0;
}

void process_command(tftp_client_t *client, char *command)
{
    if (strcmp(command, "connect") == 0)
        connect_to_server(client);
    else if (strcmp(command, "put") == 0)
        put_file_client(client);
    else if (strcmp(command, "get") == 0)
        get_file_client(client);
    else if (strcmp(command, "mode") == 0)
    {
        printf("Select Mode:\n");
        printf("1. Normal (512 bytes)\n");
        printf("2. Octet (1 byte)\n");
        printf("3. Netascii (512 with \\r before \\n)\n");
        printf("Enter choice: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        if (choice >= 1 && choice <= 3)
        {
            client->mode = choice;
            printf("Mode changed successfully\n");
        }
        else
            printf("Invalid mode\n");
    }
    else if (strcmp(command, "exit") == 0)
        disconnect_client(client);
    else
        printf("Invalid command\n");
}

void connect_to_server(tftp_client_t *client)
{
    char ip[INET_ADDRSTRLEN];

    printf("Enter Server IP: ");
    scanf("%s", ip);
    getchar();

    if (!validate_ip(ip))
    {
        printf("Invalid IP address\n");
        return;
    }

    client->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->sockfd < 0)
    {
        perror("Socket creation failed");
        return;
    }

    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip, &client->server_addr.sin_addr);

    client->server_len = sizeof(client->server_addr);
    strcpy(client->server_ip, ip);
    client->connected = 1;

    printf("Connected to server %s\n", ip);
}

void put_file_client(tftp_client_t *client)
{
    if (!client->connected)
    {
        printf("Not connected to server\n");
        return;
    }

    char filename[256];
    system("ls");
    printf("Enter filename to upload: ");
    scanf("%s", filename);
    getchar();

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("File not found\n");
        return;
    }
    fclose(fp);

    char buffer[BUFFER_SIZE];
    *(uint16_t *)buffer = htons(WRQ);
    strcpy(buffer + 2, filename);

    sendto(client->sockfd, buffer, strlen(filename) + 3, 0,
           (struct sockaddr *)&client->server_addr, client->server_len);

    printf("WRQ sent\n");

    // Wait for ACK block 0
    char ack[BUFFER_SIZE];
    int n = recvfrom(client->sockfd, ack, BUFFER_SIZE, 0,
                     NULL, NULL);

    if (n >= 4 && ntohs(*(uint16_t *)ack) == ACK)
    {
        printf("Received ACK 0\n");
        send_file(client->sockfd,
                  client->server_addr,
                  client->server_len,
                  filename,
                  client->mode);
    }
    else
    {
        printf("Did not receive proper ACK from server\n");
    }
}

void get_file_client(tftp_client_t *client)
{
    if (!client->connected)
    {
        printf("Not connected to server\n");
        return;
    }

    char filename[256];
    printf("Enter filename to download: ");
    scanf("%s", filename);
    getchar();

    char buffer[BUFFER_SIZE];
    *(uint16_t *)buffer = htons(RRQ);
    strcpy(buffer + 2, filename);

    sendto(client->sockfd, buffer, strlen(filename) + 3, 0,
           (struct sockaddr *)&client->server_addr, client->server_len);

    printf("RRQ sent\n");

    receive_file(client->sockfd, client->server_addr,
                 client->server_len, filename, client->mode);
}

void disconnect_client(tftp_client_t *client)
{
    if (client->connected)
        close(client->sockfd);

    printf("Exiting TFTP client\n");
    exit(0);
}