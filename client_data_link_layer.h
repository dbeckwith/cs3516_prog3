#include "network_layer.h"

#ifndef CLIENT_DATA_LINK_LAYER_H
#define CLIENT_DATA_LINK_LAYER_H

#define IS_ACK_FRAME(frame) ((frame).data_length == 0)

int frame_count;

int data_link_send_packet(int socket, packet_t* packet);
int data_link_recv_ack_packet(int socket);
void data_link_log_totals(int socket);

#endif
