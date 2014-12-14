#include "data_link_layer.h"

int data_link_send_frame(int socket, frame_t* frame);
int data_link_recv_frame(int socket, frame_t* frame);

int data_link_send_frame(int socket, frame_t* frame)
{
	frame_t ack_frame;	
	int bytes_sent;

	if ((bytes_sent = physical_send(socket, frame->buff, sizeof(frame->buff))) != sizeof(frame->buff))
	{
		return bytes_sent;
	}

	if (data_link_recv_frame(socket, &ack_frame) != sizeof(ack_frame.buff))
	{
		return -1;
	}

	if (ack_frame.frame.ack)
	{
		return bytes_sent;
	}

	return -1;
}

int data_link_send(int socket, char* buffer, int len)
{
	frame_t frame;
	int pos;
	unsigned int chunk_len;
	int bytes_sent;

	chunk_len = FRAME_DATA_SIZE;

	for (pos = 0; pos < len; pos += FRAME_DATA_SIZE)
	{
		if (pos + FRAME_DATA_SIZE >= len)
		{
			chunk_len = len - pos;
		}
		memcpy(frame.frame.data, buffer + pos, chunk_len);
		frame.frame.data_length = chunk_len;

		if (data_link_send_frame(socket, &frame) != sizeof(frame_t))
		{
			return -1;
		}
		bytes_sent += chunk_len;
	}
	return bytes_sent;
}

int data_link_recv_frame(int socket, frame_t* frame)
{
	frame_t ack_frame;
	int bytes_received;
	int total_received;

	total_received = 0;
	while (total_received < sizeof(frame->buff))
	{
		if ((bytes_received = physical_recv(socket, frame->buff + total_received, sizeof(frame->buff) - total_received)) <= 0) {
			return -1;
		}
		total_received += bytes_received;
	}

	if (!frame->frame.ack) {
		ack_frame.frame.ack = true;
		if (data_link_send(socket, ack_frame.buff, sizeof(ack_frame.buff)) != sizeof(ack_frame.buff)) {
			return -1;
		}
	}

	return total_received;
}

int data_link_recv(int socket, char* buffer, int len)
{
	frame_t frame;
	int bytes_received;

	if (data_link_recv_frame(socket, &frame) != sizeof(frame_t)) {
		return -1;
	}
	memcpy(buffer, frame.frame.data, bytes_received = frame.frame.data_length);
	return bytes_received;
}
