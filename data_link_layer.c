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

int data_link_send_frame(int socket, frame_t* frame)
{
	printf("%s Send Frame\n", DATA_LINK_STR);

	frame_t ack_frame;	
	int bytes_sent;

    frame->frame.ack = false;

	if ((bytes_sent = physical_send(socket, frame->buff, sizeof(frame->buff))) != sizeof(frame->buff))
	{
		return bytes_sent;
	}

    printf("frame contents: len = %d\n", frame->frame.data_length);
    int i;
    for (i = 0; i < frame->frame.data_length; i++) {
        printf("%x ", frame->frame.data[i]);
    }
    printf("\n");

    printf("receiving frame ack\n");
	if (data_link_recv_frame(socket, &ack_frame) != sizeof(ack_frame.buff))
	{
		return -1;
	}

	printf("receiving act frame%d\n", ack_frame.frame.ack);

	if (ack_frame.frame.ack)
	{
		return bytes_sent;
	}

	return -1;
}

int data_link_send(int socket, uint8_t* buffer, int len)
{
	printf("%s Send\n", DATA_LINK_STR);

	frame_t frame;
	int pos;
	unsigned int chunk_len;
	int bytes_sent;
	bytes_sent = 0;

	chunk_len = FRAME_DATA_SIZE;

	for (pos = 0; pos < len; pos += FRAME_DATA_SIZE)
	{
		if (pos + FRAME_DATA_SIZE >= len)
		{
			chunk_len = len - pos;
		}
		memcpy(frame.frame.data, buffer + pos, chunk_len);
		frame.frame.data_length = chunk_len;
		frame.frame.ack = false;
		frame.frame.eof = false;

		if (data_link_send_frame(socket, &frame) != sizeof(frame_t))
		{
			printf("datalinksendframe not equal -1\n");
			return -1;
		}
		printf("%d bytes sentincre\n",bytes_sent );
		bytes_sent += chunk_len;
	}
	printf("%dbytes sent%d\n", bytes_sent, len);
	return bytes_sent;
}

int data_link_recv_frame(int socket, frame_t* frame)
{
	printf("%s Receive Frame\n", DATA_LINK_STR);

	frame_t ack_frame;
	int bytes_received;
	int total_received;

	total_received = 0;

	ack_frame.frame.data_length = 0;
	ack_frame.frame.eof = false;

	while (total_received < sizeof(frame->buff))
	{
		if ((bytes_received = physical_recv(socket, frame->buff + total_received, sizeof(frame->buff) - total_received)) <= 0) {
			return -1;
		}
		total_received += bytes_received;
	}

	if (frame->frame.ack)
	{
		printf("ack frame data recv\n");
	}
	else
	{
	    printf("frame contents: len = %d\n", frame->frame.data_length);
	    int i;
	    for (i = 0; i < frame->frame.data_length; i++) {
	        printf("%x ", frame->frame.data[i]);
	    }
	    printf("\n");
	}

    
	if (!frame->frame.ack) {
		printf("sending frame ack\n");
		ack_frame.frame.ack = true;
		if (physical_send(socket, ack_frame.buff, sizeof(ack_frame.buff)) != sizeof(ack_frame.buff)) {
			return -1;
		}
	}

	return total_received;
}

int data_link_recv(int socket, uint8_t* buffer, int len)
{
	printf("%s Receive\n", DATA_LINK_STR);

	frame_t frame;
	int bytes_received;

	if (data_link_recv_frame(socket, &frame) != sizeof(frame_t)) {
		return -1;
	}
	memcpy(buffer, frame.frame.data, bytes_received = frame.frame.data_length);
	return bytes_received;
}
