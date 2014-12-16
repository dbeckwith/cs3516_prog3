#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util.h"
#include "data_link_layer.h"

#ifndef PHYSICAL_LAYER_H
#define PHYSICAL_LAYER_H

#define PHYSICAL_STR "[PHYSICAL LAYER]: " // String to display for debugging at physical layer

// Timer definitions
#define TIMEOUT 200
#define ERR_TIMEOUT -2

// Function definitions
int physical_connect(char *serverName, unsigned short serverPort);
int physical_error(int socket, frame_t* frame);
int physical_send_frame(int socket, frame_t* frame);
int physical_recv_frame(int socket, frame_t* frame, bool timeout);
int physical_listen(unsigned short port, unsigned int max_pending_clients);
int physical_accept(int socket, struct sockaddr_in* client_addr, unsigned int* client_len);

#endif
