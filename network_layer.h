#include "util.h"

#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#define PKT_DATA_SIZE 200

typedef struct packet
{
	char[PKT_DATA_SIZE] data;
	bool eof;
} packet_t;

int network_send(int sockfd, char* buffer, unsigned int len);
int network_recv(int sockfd, char* buffer, unsigned int len);

int network_connect(char* url, unsigned short port);

int network_listen(unsigned short port, unsigned int max_pending_clients);
int network_accept(int socketfd, struct sockaddr* client_addr, unsigned int* client_len);

#endif
