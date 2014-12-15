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
    	uint8_t eof;
    	uint8_t error_detect[2];
    } frame;
    uint8_t buff[sizeof(struct frame)];
} frame_t;

// Function definitions

int data_link_send(int socket, uint8_t* buffer, int len);
int data_link_recv(int socket, uint8_t* buffer, int len);

#endif
