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
	return 0;
}

int network_send_file(int socket, char* file_name)
{
	int read_size1;
	int read_size2;
	char read_buffer1[PKT_DATA_SIZE];
	char read_buffer2[PKT_DATA_SIZE];
	char* curr_read_buffer;
	int* curr_read_size;
	char* prev_read_buffer;
	int* prev_read_size;
	char* temp_read_buffer;
	int* temp_read_size;
	FILE* photo;
	packet_t packet;

	curr_read_buffer = read_buffer1;
	curr_read_size = &read_size1;
	prev_read_buffer = read_buffer2;
	prev_read_size = &read_size2;
	read_size2 = -1;

	if ((photo = fopen(file_name, "rb")) == NULL)
	{
		return -1;
	}

	while ((*curr_read_size = fread(curr_read_buffer, 1, PKT_DATA_SIZE, photo)) >= 0)
	{
		if (*prev_read_size != -1) {
			packet.packet.eof = *curr_read_size == 0;
			memcpy(packet.packet.data, prev_read_buffer, *prev_read_size);
			if (network_send_packet(socket, &packet) < 0){
				return -1;
			}
			if (packet.packet.eof) {
				return 0;
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
	int chunk_len;

	chunk_len = PKT_DATA_SIZE;
	for (pos = 0; pos < len; pos += PKT_DATA_SIZE) {
		packet.packet.eof = pos + PKT_DATA_SIZE >= len;
		if (packet.packet.eof) {
			chunk_len = len - pos;
		}
		memcpy(packet.packet.data, buffer + pos, chunk_len);
		if (network_send_packet(sockfd, &packet) < 0){
			return -1;
		}
	}
	return 0;
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
