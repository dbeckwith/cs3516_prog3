#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "network_layer.h"
#include "data_link_layer.h"
#include "physical_layer.h"

/*
 * @brief Send packet to data link layer
 * @param socket The socket to send the packet to
 * @param packet The packet struct that is to be sent
 * @return bytes_sent The number of bytes sent, or -1 on error
 */
int network_send_packet(int socket, packet_t* packet)
{
	packet_t ack_packet;
	unsigned int bytes_sent;

	// Check if packet is sent successfully
	if ((bytes_sent = data_link_send(socket, packet->buff, sizeof(packet->buff)) < 0) != sizeof(packet->buff))
	{
		return bytes_sent;
	}

	// Check if packet is ACKed successfully
	if (data_link_recv(socket, &ack_packet.buff, sizeof(ack_packet.buff)) != sizeof(ack_packet.buff))
	{
		return -1;
	}

	// Check if returned ACK is valid ACK packet
	if (ack_packet.packet.ack)
	{
		return bytes_sent;
	}
	return -1;
}

/*
 * @brief Send packets from reading chunks of the given photo file
 * @param socket The socket to send the packet to
 * @param file_name The photo file to open and read
 * @return bytes_sent The number of bytes sent, or -1 on error
 */
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

	/*
	 * Read from file, and then read again into new buffer.
	 * Compare previous and current reads and determine if the previous packet is the last packet of a photo.
	 */
	while ((*curr_read_size = fread(curr_read_buffer, 1, PKT_DATA_SIZE, photo)) >= 0)
	{
		if (*prev_read_size != -1)
		{
			packet.packet.eof = *curr_read_size == 0;
			memcpy(packet.packet.data, prev_read_buffer, *prev_read_size);
			packet.packet.data_length = *prev_read_size;

			if (network_send_packet(socket, &packet) != sizeof(packet))
			{
				return -1;
			}

			bytes_sent += *prev_read_size;

			if (packet.packet.eof)
			{
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

/*
 * @brief Send a buffer of some length to the socket
 * @param socket The socket to send the packet to
 * @param buffer The buffer to be sent
 * @param len The length of given buffer
 * @return bytes_sent The number of bytes sent, or -1 on error
 */
int network_send(int socket, char* buffer, unsigned int len)
{
	packet_t packet;
	unsigned int pos;
	unsigned int chunk_len;
	unsigned int bytes_sent;

	chunk_len = PKT_DATA_SIZE;
	for (pos = 0; pos < len; pos += PKT_DATA_SIZE)
	{
		packet.packet.eof = pos + PKT_DATA_SIZE >= len;

		if (packet.packet.eof)
		{
			chunk_len = len - pos;
		}
		memcpy(packet.packet.data, buffer + pos, chunk_len);
		packet.packet.data_length = chunk_len;
		
		if (network_send_packet(socket, &packet) != sizeof(packet))
		{
			return -1;
		}
		bytes_sent += chunk_len;
	}
	return bytes_sent;
}

int network_recv_packet(int socket, packet_t* packet)
{
	// TODO: This function
}

int network_recv(int socket, char* buffer, unsigned int len)
{
	// TODO: This function
}

/*
 * @brief Call physical layer connect on given url and port
 * @param url The server name to connect to
 * @param port The port that the server is listening on
 * @return Server socket that is connected
 */
int network_connect(char* url, unsigned short port)
{
	return physical_connect(url, port);
}

/*
 * @brief Call physical layer listen for server port
 * @param port The port to listen on
 * @param max_pending_clients The max number of clients to keep in the connection queue
 * @return Server socket that is being listened to
 */
int network_listen(unsigned short port, unsigned int max_pending_clients)
{
	return physical_listen(port, max_pending_clients);
}

/*
 * @brief Call physical layer accept on new client
 * @param socket The socket descriptor to accept
 * @param client_addr The client address struct to accept
 * @param client_len The length of the client address being given
 * @return The client socket that is accepted
 */
int network_accept(int socket, struct sockaddr* client_addr, unsigned int* client_len)
{
	return physical_accept(socket, client_addr, client_len);
}
