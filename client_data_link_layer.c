#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "util.h"
#include "network_layer.h"
#include "client_network_layer.h"
#include "data_link_layer.h"
#include "client_data_link_layer.h"
#include "physical_layer.h"

static int frame_retransmissions = 0;
static int frames_sent = 0;
static int good_acks = 0;
static int bad_acks = 0;

/*
 * @brief Send packet to data link layer in frames
 * @param socket The socket to send the frames to
 * @param packet The packet that is to be sent
 * @return The size of the packet sent, or -1 on error
 */
int data_link_send_packet(int socket, packet_t* packet)
{
    frame_t frame;
    size_t packet_size;
    unsigned int pos;
    unsigned int chunk_len;
	static seq_t curr_seq_num = 0;
    int recv_err;
    bool last_frame;
    bool first_run;

    packet_size = sizeof(packet_t);
    frame_count = 0;

    chunk_len = FRAME_DATA_SIZE;
    last_frame = false;

    // Split packet into individual frames to send to server
    for (pos = 0; pos < packet_size; pos += FRAME_DATA_SIZE)
    {
        if (pos + FRAME_DATA_SIZE >= packet_size)
        {
            // Chunk length is just to end of packet
            chunk_len = packet_size - pos;
            last_frame = true;
        }
        first_run = true;
        while (true)
        {
            // Copy packet bytes into frame data
            memcpy(frame.frame.data, packet->bytes + pos, chunk_len);
            frame.frame.data_length = chunk_len;
            frame.frame.seq_num = curr_seq_num;
            frame.frame.eof = last_frame;
            frame.frame.chksum = gen_chksum(&frame);

	        // Send frame through physical layer
	        DEBUG(DATA_LINK_STR "sending frame through physical layer\n");
            if (!first_run)
            {
                photo_log(socket, "Frame %d of packet %d being resent.\n", frame_count, packet_count);
                frame_retransmissions++;
            }
            first_run = false;

            // Incomplete physical send. Bad error
	        if (physical_send_frame(socket, &frame) != sizeof(frame_t))
	        {
	            return -1;
	        }

            frame_count++;
	        photo_log(socket, "Frame %d of packet %d sent successfully.\n", frame_count, packet_count);
            frames_sent++;

	        // Wait for ACK frame
	        DEBUG(DATA_LINK_STR "waiting for frame ack through physical layer\n");
	        if ((recv_err = physical_recv_frame(socket, &frame, true)) != sizeof(frame_t))
            {
	            DEBUG(DATA_LINK_STR "error receiving frame ack\n");
                // Timer times out
	            if (recv_err == ERR_TIMEOUT)
                {
	            	DEBUG(DATA_LINK_STR "timeout waiting for ack frame\n");
                    photo_log(socket, "Frame %d ACK timed out\n", frame_count);
	            	continue;
	            }
	            return -1;
	        }

            // Check sum error
            if (frame.frame.chksum != gen_chksum(&frame))
            {
                DEBUG(DATA_LINK_STR "frame ack checksum failed\n");
                photo_log(socket, "Frame %d ACK checksum failed\n", frame_count);
                bad_acks++;
                continue;
            }

            // Invalid ACK frame
	        if (!IS_ACK_FRAME(frame.frame))
            {
	            DEBUG(DATA_LINK_STR "frame ack was not an ack\n");
	            return -1;
	        }

	        // Check ACK's seq num
	        if (frame.frame.seq_num != curr_seq_num)
            {
            	DEBUG(DATA_LINK_STR "ack frame was wrong sequence number\n");
                photo_log(socket, "Frame %d ACK sequence error\n", frame_count);
                bad_acks++;
	        	continue;
	        }

	        DEBUG(DATA_LINK_STR "ack frame accepted\n");
            photo_log(socket, "Frame %d ACKed successfully.\n", frame_count);
            good_acks++;
	        break;
	    }

        INC_SEQ(curr_seq_num); // Increment sequence number
    }
    return sizeof(packet_t);
}

/*
 * @brief Receive buffer from data link layer in frames
 * @param socket The socket to receive the frame from
 * @return 0 on success, -1 on error
 */
int data_link_recv_ack_packet(int socket)
{
    frame_t frame;

    if (physical_recv_frame(socket, &frame, true) != sizeof(frame_t)) {
        return -1;
    }
    photo_log(socket, "ACK packet frame %d received successfully.\n", packet_count);
    if (!IS_ACK_FRAME(frame.frame)) {
        return -1;
    }
    return 0;
}

/*
 * @brief Log frame performance totals to log file
 */
void data_link_log_totals(int socket)
{
    photo_log(socket, "Frame retransmissions: %d, Frames sent: %d, Good ACKs: %d, Bad ACKs: %d\n", frame_retransmissions, frames_sent, good_acks, bad_acks);
}
