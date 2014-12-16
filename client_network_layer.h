#include "util.h"
#include <stdint.h>

#ifndef CLIENT_NETWORK_LAYER_H
#define CLIENT_NETWORK_LAYER_H

// ACK packet macro
#define IS_ACK_PACKET(packet) ((packet).data_length == 0)

int packet_count;

// Function definitions
int network_send_file(int socket, char* file_name);
int network_send(int socket, uint8_t* data, size_t data_size);

#endif
