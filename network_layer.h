#include "util.h"

#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#define NETWORK_STR "[NETWORK LAYER]" // String to display for debugging at network layer
#define PKT_DATA_SIZE 200

// Union defintion for packet for easy conversion to bytes to pass as frames
typedef union
{
    struct packet
    {
        char ack;
    	char data[PKT_DATA_SIZE];
    	char data_length;
    	char eof; // End of file indication
    } packet;
    char buff[sizeof(struct packet)];
} packet_t;

// Function definitions

int network_send_file(int socket, char* file_name);
int network_send(int socket, char* buffer, unsigned int len);
int network_recv_file(int socket, char* file_name);
int network_recv(int socket, char* buffer, unsigned int len);
int network_connect(char* url, unsigned short port);
int network_listen(unsigned short port, unsigned int max_pending_clients);
int network_accept(int socket, struct sockaddr* client_addr, unsigned int* client_len);

#endif
