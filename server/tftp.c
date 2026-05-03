#include "tftp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename, int mode)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        perror("File open failed");
        return;
    }

    char buffer[BUFFER_SIZE];
    uint16_t block = 1;
    size_t bytes_read;
    int retries;

    printf("Starting file transfer: %s\n", filename);

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        // Set DATA opcode
        *(uint16_t *)buffer = htons(DATA);
        *(uint16_t *)(buffer + 2) = htons(block);

        if (mode == MODE_OCTET)
        {
            bytes_read = fread(buffer + 4, 1, 1, fp);
        }
        else if (mode == MODE_NETASCII)
        {
            char temp[512];
            bytes_read = fread(temp, 1, 512, fp);
            int j = 0;
            for (int i = 0; i < bytes_read; i++)
            {
                if (temp[i] == '\n')
                {
                    buffer[4 + j++] = '\r';
                }
                buffer[4 + j++] = temp[i];
            }
            bytes_read = j;
        }
        else
        {
            bytes_read = fread(buffer + 4, 1, 512, fp);
        }

        retries = 0;

        while (retries < MAX_RETRIES)
        {
            sendto(sockfd, buffer, bytes_read + 4, 0,
                   (struct sockaddr *)&client_addr, client_len);

            printf("Sent block %d\n", block);

            char ack_buf[BUFFER_SIZE];
            int n = recvfrom(sockfd, ack_buf, BUFFER_SIZE, 0,
                             (struct sockaddr *)&client_addr, &client_len);

            if (n >= 4)
            {
                uint16_t opcode = ntohs(*(uint16_t *)ack_buf);
                uint16_t ack_block = ntohs(*(uint16_t *)(ack_buf + 2));

                if (opcode == ACK && ack_block == block)
                {
                    printf("Received ACK %d\n", block);
                    break;
                }
            }

            retries++;
            printf("Retry %d for block %d\n", retries, block);
        }

        if (retries == MAX_RETRIES)
        {
            printf("Transfer failed: Max retries reached\n");
            fclose(fp);
            return;
        }

        if (bytes_read < 512)
            break;

        block++;
    }

    printf("File transfer completed\n");
    fclose(fp);
}

void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename, int mode)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        perror("File create failed");
        return;
    }

    char buffer[BUFFER_SIZE];
    uint16_t expected_block = 1;
    int retries = 0;

    printf("Receiving file: %s\n", filename);

    while (1)
    {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&client_addr, &client_len);

        if (n < 4)
        {
            retries++;
            if (retries == MAX_RETRIES)
            {
                printf("Receive failed: Max retries reached\n");
                fclose(fp);
                return;
            }
            continue;
        }

        uint16_t opcode = ntohs(*(uint16_t *)buffer);
        uint16_t block = ntohs(*(uint16_t *)(buffer + 2));

        if (opcode == DATA && block == expected_block)
        {
            fwrite(buffer + 4, 1, n - 4, fp);

            char ack[4];
            *(uint16_t *)ack = htons(ACK);
            *(uint16_t *)(ack + 2) = htons(block);

            sendto(sockfd, ack, 4, 0,
                   (struct sockaddr *)&client_addr, client_len);

            printf("Received block %d\n", block);

            if (n - 4 < 512)
                break;

            expected_block++;
            retries = 0;
        }
    }

    printf("File received successfully\n");
    fclose(fp);
}