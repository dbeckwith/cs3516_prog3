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

int physical_error(int socket, frame_t* frame)
{
    photo_log(socket, "Server error function called.\n");
}
