#include <stdint.h>

#ifndef DATA_LINK_LAYER_H
#define DATA_LINK_LAYER_H

#define DATA_LINK_STR "[DATA LINK LAYER]: " // String to display for debugging at data link layer
#define FRAME_DATA_SIZE 124

#define SEQ_LEN 2
#define INC_SEQ(seq) ((seq) = ((seq) + 1) % SEQ_LEN)
typedef uint16_t seq_t;

// Union definition for frame for easy conversion to bytes to send
typedef union
{
    struct frame
    {
        seq_t seq_num;
        uint8_t data_length;
        uint8_t data[FRAME_DATA_SIZE];
        uint16_t chksum;
    } frame;
    uint8_t bytes[sizeof(struct frame)];
} frame_t;

uint16_t gen_chksum(frame_t* frame);

#endif
