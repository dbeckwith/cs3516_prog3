#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#include "network_layer.h"
#include "client_network_layer.h"
#include "data_link_layer.h"
#include "client_data_link_layer.h"
#include "physical_layer.h"

/*
 * @brief Send buffer to data link layer in frames
 * @param socket The socket to send the frames to
 * @param buffer The buffer that is to be sent
 * @param buffer_size The length of the given buffer
 * @return bytes_sent The number of bytes sent, or -1 on error
 */
int data_link_send_packet(int socket, packet_t* packet)
{
    frame_t frame;
    size_t packet_size;
    unsigned int pos;
    unsigned int chunk_len;

    packet_size = sizeof(packet_t);
    frame_count = 0;

    chunk_len = FRAME_DATA_SIZE;

    // split packet into individual frames to send to server
    for (pos = 0; pos < packet_size; pos += FRAME_DATA_SIZE)
    {
        if (pos + FRAME_DATA_SIZE >= packet_size)
        {
            // chunk length is just to end of packet
            chunk_len = packet_size - pos;
        }
        // copy packet bytes into frame data
        memcpy(frame.frame.data, packet->bytes + pos, chunk_len);
        frame.frame.data_length = chunk_len;

        // send frame through physical layer
        if (DEBUG) printf(DATA_LINK_STR "sending frame through physical layer\n");
        if (physical_send_frame(socket, &frame) != sizeof(frame_t))
        {
            return -1;
        }

        photo_log(socket, "Frame %d of packet %d sent successfully.\n", ++frame_count, packet_count);

        // wait for ACK frame
        if (DEBUG) printf(DATA_LINK_STR "waiting for frame ack through physical layer\n");
        if (physical_recv_frame(socket, &frame) != sizeof(frame_t)) {
            if (DEBUG) printf(DATA_LINK_STR "frame ack was wrong size\n");
            return -1;
        }
        if (!IS_ACK_FRAME(frame.frame)) {
            if (DEBUG) printf(DATA_LINK_STR "frame ack was not an ack\n");
            return -1;
        }
    }
    return sizeof(packet_t);
}

/*
 * @brief Receive buffer from data link layer in frames
 * @param socket The socket to receive the frame from
 * @param buffer The buffer that is to be received
 * @param buffer_size The length of the given buffer
 * @return bytes_received The number of bytes received, or -1 on error
 */
int data_link_recv_ack_packet(int socket)
{
    frame_t frame;

    if (physical_recv_frame(socket, &frame) != sizeof(frame_t)) {
        return -1;
    }
    photo_log(socket, "ACK packet frame %d received successfully.\n", packet_count);
    if (!IS_ACK_FRAME(frame.frame)) {
        return -1;
    }
    return 0;
}
