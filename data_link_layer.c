#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#include "data_link_layer.h"
#include "physical_layer.h"

int data_link_send_frame(int socket, frame_t* frame);
int data_link_recv_frame(int socket, frame_t* frame);

/*
 * @brief Send frame to physical layer
 * @param socket The socket to send the frame to
 * @param frame The frame struct that is to be sent
 * @return bytes_sent The number of bytes sent, or -1 on error
 */
int data_link_send_frame(int socket, frame_t* frame)
{
	frame_t ack_frame;	
	int bytes_sent;

    frame->frame.ack = false;

	if ((bytes_sent = physical_send(socket, frame->bytes, sizeof(frame->bytes))) != sizeof(frame->bytes))
	{
		return bytes_sent;
	}

	if (data_link_recv_frame(socket, &ack_frame) != sizeof(ack_frame.bytes))
	{
		return -1;
	}

	if (ack_frame.frame.ack)
	{
		return bytes_sent;
	}

	return -1;
}

/*
 * @brief Send buffer to data link layer in frames
 * @param socket The socket to send the frames to
 * @param buffer The buffer that is to be sent
 * @param buffer_size The length of the given buffer
 * @return bytes_sent The number of bytes sent, or -1 on error
 */
int data_link_send(int socket, uint8_t* buffer, unsigned int buffer_size)
{
	frame_t frame;
	unsigned int pos;
	unsigned int chunk_len;
	int bytes_sent;
	bytes_sent = 0;

	chunk_len = FRAME_DATA_SIZE;

	for (pos = 0; pos < buffer_size; pos += FRAME_DATA_SIZE)
	{
		if (pos + FRAME_DATA_SIZE >= buffer_size)
		{
			chunk_len = buffer_size - pos;
		}
		memcpy(frame.frame.data, buffer + pos, chunk_len);
		frame.frame.data_length = chunk_len;
		frame.frame.ack = false;
		frame.frame.eof = false;

		if (data_link_send_frame(socket, &frame) != sizeof(frame_t))
		{
			return -1;
		}
		bytes_sent += chunk_len;
	}
	return bytes_sent;
}

/*
 * @brief Receive frame from data link layer
 * @param socket The socket to receive the frame from
 * @param frame The frame struct that is to be received
 * @return total_received The number of bytes received, or -1 on error
 */
int data_link_recv_frame(int socket, frame_t* frame)
{
	frame_t ack_frame;
	int bytes_received;
	int total_received;

	total_received = 0;

	ack_frame.frame.data_length = 0;
	ack_frame.frame.eof = false;

	while (total_received < sizeof(frame->bytes))
	{
		if ((bytes_received = physical_recv(socket, frame->bytes + total_received, sizeof(frame->bytes) - total_received)) <= 0) {
			return -1;
		}
		total_received += bytes_received;
	}
    
	if (!frame->frame.ack) {
		ack_frame.frame.ack = true;
		if (physical_send(socket, ack_frame.bytes, sizeof(ack_frame.bytes)) != sizeof(ack_frame.bytes)) {
			return -1;
		}
	}

	return total_received;
}

/*
 * @brief Receive buffer from data link layer in frames
 * @param socket The socket to receive the frame from
 * @param buffer The buffer that is to be received
 * @param buffer_size The length of the given buffer
 * @return bytes_received The number of bytes received, or -1 on error
 */
int data_link_recv(int socket, uint8_t* buffer, unsigned int buffer_size)
{
	frame_t frame;
	int bytes_received;

	if (data_link_recv_frame(socket, &frame) != sizeof(frame_t)) {
		return -1;
	}
	bytes_received = frame.frame.data_length;
	memcpy(buffer, frame.frame.data, bytes_received > buffer_size? buffer_size : bytes_received);
	return bytes_received;
}
