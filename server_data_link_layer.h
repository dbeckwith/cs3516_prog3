#include "network_layer.h"

#ifndef SERVER_DATA_LINK_LAYER_H
#define SERVER_DATA_LINK_LAYER_H

// Function definitions
int data_link_send_ack_packet(int socket);
int data_link_recv_packet(int socket, packet_t* packet);

#endif
