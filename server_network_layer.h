#include "util.h"
#include <stdint.h>

#ifndef SERVER_NETWORK_LAYER_H
#define SERVER_NETWORK_LAYER_H

// Function definitions

int network_recv_file(int socket, char* file_name);
int network_recv(int socket, uint8_t* data, size_t data_size);

#endif
