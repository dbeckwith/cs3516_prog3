#include <stdint.h>
#include "data_link_layer.h"

uint16_t gen_chksum(frame_t* frame) {
    uint32_t hash;
    int i;
    hash = 2166136261;
    for (i = 0; i < sizeof(frame_t) - sizeof(uint16_t); i++) {
        hash *= 16777619;
        hash ^= frame->bytes[i];
    }
    return (uint16_t)hash;
}
