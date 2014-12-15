#include "network_layer.h"

#ifndef CLIENT_DATA_LINK_LAYER_H
#define CLIENT_DATA_LINK_LAYER_H

int data_link_send_packet(int socket, packet_t* packet);
int data_link_recv_ack_packet(int socket);

#endif
