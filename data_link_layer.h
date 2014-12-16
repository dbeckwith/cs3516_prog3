#include <stdint.h>

#ifndef DATA_LINK_LAYER_H
#define DATA_LINK_LAYER_H

#define DATA_LINK_STR "[DATA LINK LAYER]: " // String to display for debugging at data link layer
#define FRAME_DATA_SIZE 124

// Union definition for frame for easy conversion to bytes to send
typedef union
{
    struct frame
    {
    	uint8_t ack;
    	uint8_t seq_num[2];
    	uint8_t data[FRAME_DATA_SIZE];
    	uint8_t data_length;
    	uint8_t error_detect[2];
    } frame;
    uint8_t bytes[sizeof(struct frame)];
} frame_t;

#endif
