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

// Error definitions
#define ERROR_BIT 7
#define ERROR_MASK (1 << ERROR_BIT)

/*
 * @brief Function for creating errors on every 5th frame on the client side
 * @param socket The socket to log to
 * @param frame The frame to generate errors on
 * @author djbeckwith
 */
int physical_error(int socket, frame_t* frame)
{
    static int frame_count = 0;
    frame_count++;

    if ((frame_count %= 5) == 0) {
        DEBUG(PHYSICAL_STR "Client error induced. ZAP\n");
        photo_log(socket, "Client error induced. ZAP\n");
        frame->frame.chksum ^= ERROR_MASK;
    }
}
