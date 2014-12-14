#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "network_layer.h"
#include "data_link_layer.h"
#include "physical_layer.h"

int network_send_packet(int socket, packet_t* packet)
{
	packet_t ack_packet;
	unsigned int bytes_sent;

	if ((bytes_sent = data_link_send(socket, packet.buff, sizeof(packet.buff)) < 0) != sizeof(packet.buff)) {
		return bytes_sent;
	}
	if (data_link_recv(socket, &ack_packet.buff, sizeof(ack_packet.buff)) != sizeof(ack_packet.buff)) {
		return -1;
	}
	if (ack_packet.ack) {
		return bytes_sent;
	}
	return -1;
}

int network_send_file(int socket, char* file_name)
{
	unsigned int read_size1;
	unsigned int read_size2;
	char read_buffer1[PKT_DATA_SIZE];
	char read_buffer2[PKT_DATA_SIZE];
	char* curr_read_buffer;
	unsigned int* curr_read_size;
	char* prev_read_buffer;
	unsigned int* prev_read_size;
	char* temp_read_buffer;
	unsigned int* temp_read_size;
	FILE* photo;
	unsigned int bytes_sent;
	packet_t packet;

	curr_read_buffer = read_buffer1;
	curr_read_size = &read_size1;
	prev_read_buffer = read_buffer2;
	prev_read_size = &read_size2;
	read_size2 = -1;
	bytes_sent = 0;

	if ((photo = fopen(file_name, "rb")) == NULL)
	{
		return -1;
	}

	while ((*curr_read_size = fread(curr_read_buffer, 1, PKT_DATA_SIZE, photo)) >= 0)
	{
		if (*prev_read_size != -1) {
			packet.packet.eof = *curr_read_size == 0;
			memcpy(packet.packet.data, prev_read_buffer, *prev_read_size);
			packet.packet.data_length = *prev_read_size;
			if (network_send_packet(socket, &packet) != sizeof(packet)) {
				return -1;
			}
			bytes_sent += *prev_read_size;
			if (packet.packet.eof) {
				return bytes_sent;
			}
		}
		temp_read_buffer = curr_read_buffer;
		temp_read_size = curr_read_size;
		curr_read_buffer = prev_read_buffer;
		curr_read_size = prev_read_size;
		prev_read_buffer = temp_read_buffer;
		prev_read_size = temp_read_size;
	}
	return -1;
}

int network_send(int sockfd, char* buffer, unsigned int len)
{
	packet_t packet;
	unsigned int pos;
	unsigned int chunk_len;
	unsigned int bytes_sent;

	chunk_len = PKT_DATA_SIZE;
	for (pos = 0; pos < len; pos += PKT_DATA_SIZE) {
		packet.packet.eof = pos + PKT_DATA_SIZE >= len;
		if (packet.packet.eof) {
			chunk_len = len - pos;
		}
		memcpy(packet.packet.data, buffer + pos, chunk_len);
		packet.packet.data_length = chunk_len;
		if (network_send_packet(sockfd, &packet) != sizeof(packet)){
			return -1;
		}
		bytes_sent += chunk_len;
	}
	return bytes_sent;
}

int network_recv_packet(int socket, packet_t* packet)
{
	
}

int network_recv(int sockfd, char* buffer, unsigned int len)
{

}

int network_connect(char* url, unsigned short port)
{
	return physical_connect(url, port);
}

int network_listen(unsigned short port, unsigned int max_pending_clients)
{
	return physical_listen(port, max_pending_clients);
}

int network_accept(int socketfd, struct sockaddr* client_addr, unsigned int* client_len)
{
	return physical_accept(socketfd, client_addr, client_len);
}
