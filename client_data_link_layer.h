#include "network_layer.h"

#ifndef CLIENT_DATA_LINK_LAYER_H
#define CLIENT_DATA_LINK_LAYER_H

// ACK frame macro
#define IS_ACK_FRAME(frame) ((frame).data_length == 0)

int frame_count; // Current frame number for each packet

// Function declarations
int data_link_send_packet(int socket, packet_t* packet);
int data_link_recv_ack_packet(int socket);
void data_link_log_totals(int socket);

#endif
