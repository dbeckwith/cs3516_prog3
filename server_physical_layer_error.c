#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#include "photo.h"
#include "physical_layer.h"

#define ERROR_BIT 11
#define ERROR_MASK (1 << ERROR_BIT)

int physical_error(int socket, frame_t* frame)
{

    static int frame_count = 0;
    frame_count++;

    if ((frame_count %= 13) == 0) {
        DEBUG(PHYSICAL_STR "Server error induced. ZAP\n");
        photo_log(socket, "Server error induced. ZAP\n");
        frame->frame.chksum ^= ERROR_MASK;
    }
}
