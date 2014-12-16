#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#include "network_layer.h"
#include "server_network_layer.h"
#include "data_link_layer.h"
#include "server_data_link_layer.h"
#include "physical_layer.h"

/*
 * @brief Send buffer to data link layer in frames
 * @param socket The socket to send the frames to
 * @param buffer The buffer that is to be sent
 * @param buffer_size The length of the given buffer
 * @return bytes_sent The number of bytes sent, or -1 on error
 */
int data_link_send_ack_packet(int socket)
{
    frame_t frame;
    frame.frame.data_length = 0;

    if (physical_send_frame(socket, &frame) != sizeof(frame_t))
    {
        return -1;
    }
    photo_log(socket, "ACK frame sent successfully.\n");

    return 0;
}

/*
 * @brief Receive frame from data link layer
 * @param socket The socket to receive the frame from
 * @param frame The frame struct that is to be received
 * @return total_received The number of bytes received, or -1 on error
 */
int data_link_recv_packet(int socket, packet_t* packet)
{
    int pos;
    unsigned int chunk_len;
    size_t packet_size;
    frame_t frame;
    static seq_t curr_seq_num = 0;

    packet_size = sizeof(packet_t);

    for (pos = 0; pos < packet_size; pos += chunk_len)
    {
    	while (true) {
	        if (physical_recv_frame(socket, &frame) != sizeof(frame_t))
	        {
	            return -1;
	        }
	        if (frame.frame.seq_num != curr_seq_num) {
	        	continue;
	        }
	        break;
	    }

        photo_log(socket, "Frame received successfully.\n");

        chunk_len = frame.frame.data_length;
        if (pos + chunk_len > packet_size)
        {
            return -1;
        }
        memcpy(packet->bytes + pos, frame.frame.data, chunk_len);

        frame.frame.data_length = 0;
        memcpy(&frame.frame.seq_num, &curr_seq_num, sizeof(curr_seq_num));

        if (physical_send_frame(socket, &frame) != sizeof(frame_t))
        {
            return -1;
        }
        photo_log(socket, "ACK frame sent successfully.\n");

        INC_SEQ(curr_seq_num);
    }

    photo_log(socket, "Packet sent to network layer successfully.\n");
    return sizeof(packet_t);
}
