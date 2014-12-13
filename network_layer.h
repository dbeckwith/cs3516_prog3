#include "util.h"

#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#define PKT_DATA_SIZE 200

typedef union {
    struct packet {
    	char data[PKT_DATA_SIZE];
    	char data_length;
    	char eof;
        char ack;
    } packet;
    char buff[sizeof(struct packet)];
} packet_t;

int network_send_file(int socket, char* file_name);

int network_send(int sockfd, char* buffer, unsigned int len);
int network_recv(int sockfd, char* buffer, unsigned int len);

int network_connect(char* url, unsigned short port);

int network_listen(unsigned short port, unsigned int max_pending_clients);
int network_accept(int socketfd, struct sockaddr* client_addr, unsigned int* client_len);

#endif
