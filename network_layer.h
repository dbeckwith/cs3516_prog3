#include "util.h"
#include <stdint.h>

#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#define NETWORK_STR "[NETWORK LAYER]: " // String to display for debugging at network layer
#define PKT_DATA_SIZE 200

// Union defintion for packet for easy conversion to bytes to pass as frames
typedef union
{
    struct packet
    {
        uint8_t data[PKT_DATA_SIZE];
        uint8_t data_length;
        uint8_t eof; // End of file indication
    } packet;
    uint8_t bytes[sizeof(struct packet)];
} packet_t;

#endif
