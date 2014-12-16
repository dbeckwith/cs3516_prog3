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
 * @return 0 on success, or -1 on error
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
 * @param packet The frame struct that is to be received
 * @return The number of bytes received, or -1 on error
 */
int data_link_recv_packet(int socket, packet_t* packet)
{
    int pos;
    unsigned int chunk_len;
    size_t packet_size;
    frame_t frame;
    frame_t prev_frame;
    static seq_t curr_seq_num = 0;

    packet_size = sizeof(packet_t);

    for (pos = 0; pos < packet_size; pos += chunk_len)
    {
    	while (true) {
            DEBUG(DATA_LINK_STR "Receiving physical frame\n");
            // Receive frame
	        if (physical_recv_frame(socket, &frame, false) != sizeof(frame_t))
	        {
                DEBUG(DATA_LINK_STR "Error receiving data frame\n");
	            return -1;
	        }

            DEBUG(DATA_LINK_STR "Checking checksum\n");
            // Check checksum
            if (frame.frame.chksum != gen_chksum(&frame)) {
                photo_log(socket, "Frame checksum error detected.\n");
                continue;
            }

            DEBUG(DATA_LINK_STR "Checking seq num\n");
            // Check sequence number
	        if (frame.frame.seq_num != curr_seq_num) {
                DEBUG(DATA_LINK_STR "Frame sequence number error detected\n");
                photo_log(socket, "Frame sequence number error detected.\n");
                if (pos == 0) {
                    DEBUG(DATA_LINK_STR "Frame sequence mismatch error detected\n");
                    return -1;
                }
                else if (memcmp(&frame, &prev_frame, sizeof(frame_t)) == 0) {
                    DEBUG(DATA_LINK_STR "Duplicate frame detected\n");
                    photo_log(socket, "Duplicate frame detected.\n");

                    // Send duplicate ACK
                    frame.frame.data_length = 0;
                    // Same seq number as just received frame, the duplicate
                    frame.frame.chksum = gen_chksum(&frame);

                    // Resend ACK frame
                    if (physical_send_frame(socket, &frame) != sizeof(frame_t))
                    {
                        DEBUG(DATA_LINK_STR "Error resending ACK frame\n");
                        return -1;
                    }
                    photo_log(socket, "ACK frame resent successfully.\n");
                }
	        	continue;
	        }
	        break;
	    }

        DEBUG(DATA_LINK_STR "Frame received successfully\n");
        photo_log(socket, "Frame received successfully.\n");

        chunk_len = frame.frame.data_length;
        if (pos + chunk_len > packet_size)
        {
            DEBUG(DATA_LINK_STR "ERROR: Received too many bytes to make a packet\n");
            return -1;
        }
        memcpy(packet->bytes + pos, frame.frame.data, chunk_len);

        memcpy(&prev_frame, &frame, sizeof(frame_t));

        frame.frame.data_length = 0;
        frame.frame.seq_num = curr_seq_num;
        frame.frame.chksum = gen_chksum(&frame);

        // Send ACK frame
        if (physical_send_frame(socket, &frame) != sizeof(frame_t))
        {
            DEBUG(DATA_LINK_STR "Error sending ACK frame\n");
            return -1;
        }
        photo_log(socket, "ACK frame sent successfully.\n");

        INC_SEQ(curr_seq_num);
        
        // Check for EOF
        if (frame.frame.eof) {
            break;
        }
    }

    photo_log(socket, "Packet sent to network layer successfully.\n");
    return sizeof(packet_t);
}
