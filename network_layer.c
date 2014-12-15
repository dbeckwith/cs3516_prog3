#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "network_layer.h"
#include "data_link_layer.h"
#include "physical_layer.h"

int network_send_packet(int socket, packet_t* packet);
int network_recv_packet(int socket, packet_t* packet);

/*
 * @brief Send packet to data link layer
 * @param socket The socket to send the packet to
 * @param packet The packet struct that is to be sent
 * @return bytes_sent The number of bytes sent, or -1 on error
 */
int network_send_packet(int socket, packet_t* packet)
{
	printf("%s Send Packet\n", NETWORK_STR);

	packet_t ack_packet;
	int bytes_sent;

	// Check if packet is sent successfully
	if ((bytes_sent = data_link_send(socket, packet->buff, sizeof(packet->buff))) != sizeof(packet->buff))
	{
		return bytes_sent;
	}

    printf("packet contents: len = %d\n", packet->packet.data_length);
    int i;
    for (i = 0; i < packet->packet.data_length; i++) {
        printf("%x ", packet->packet.data[i]);
    }
    printf("\n");

	printf("receiving packet ack\n");
	// Check if packet is ACKed successfully
	if (network_recv_packet(socket, &ack_packet) != sizeof(ack_packet.buff))
	{
		printf("networkrecvpacketack -1\n");
		return -1;
	}

	// Check if returned ACK is valid ACK packet
	if (ack_packet.packet.ack)
	{
		printf("got ack network send packet\n");
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
int network_send_file(int socket, uint8_t* file_name)
{
	printf("%s Send File\n", NETWORK_STR);

	unsigned int read_size1;
	unsigned int read_size2;
	uint8_t read_buffer1[PKT_DATA_SIZE];
	uint8_t read_buffer2[PKT_DATA_SIZE];
	uint8_t* curr_read_buffer;
	unsigned int* curr_read_size;
	uint8_t* prev_read_buffer;
	unsigned int* prev_read_size;
	uint8_t* temp_read_buffer;
	unsigned int* temp_read_size;
	FILE* photo;
	int bytes_sent;
	bytes_sent = 0;
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
			packet.packet.ack = false;

			if (network_send_packet(socket, &packet) != sizeof(packet))
			{
				return -1;
			}

			bytes_sent += *prev_read_size;

			if (packet.packet.eof)
			{
				if (fclose(photo) < 0)
				{
					return -1;
				}
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
int network_send(int socket, uint8_t* buffer, unsigned int len)
{
	printf("%s Send\n", NETWORK_STR);

	packet_t packet;
	unsigned int pos;
	unsigned int chunk_len;
	int bytes_sent;
	bytes_sent = 0;

	chunk_len = PKT_DATA_SIZE;
	for (pos = 0; pos < len; pos += PKT_DATA_SIZE)
	{
		if (pos + PKT_DATA_SIZE >= len)
		{
			chunk_len = len - pos;
		}
		memcpy(packet.packet.data, buffer + pos, chunk_len);
		packet.packet.data_length = chunk_len;
		packet.packet.eof = false;
		packet.packet.ack = false;
		
		if (network_send_packet(socket, &packet) != sizeof(packet_t))
		{
			printf("networksentpacket -1\n");
			return -1;
		}
		bytes_sent += chunk_len;
	}
	return bytes_sent;
}

int network_recv_packet(int socket, packet_t* packet)
{
	printf("%s Receive Packet\n", NETWORK_STR);

	packet_t ack_packet;
	int bytes_received;
	int total_received;
	total_received = 0;
	ack_packet.packet.ack = true;
	ack_packet.packet.eof = false;
	ack_packet.packet.data_length = 0;

	while (total_received < sizeof(packet->buff)) {
		if ((bytes_received = data_link_recv(socket, packet->buff + total_received, sizeof(packet->buff) - total_received)) <= 0) {
			return -1;
		}
		total_received += bytes_received;
	}

	if (packet->packet.ack)
	{
		printf("ack frame data recv\n");
	}
	else
	{
	    printf("packet contents: len = %d\n", packet->packet.data_length);
	    int i;
	    for (i = 0; i < packet->packet.data_length; i++) {
	        printf("%x ", packet->packet.data[i]);
	    }
	    printf("\n");
	}

	printf("sending packet ack\n");
	if (!packet->packet.ack) {
		if (data_link_send(socket, ack_packet.buff, sizeof(ack_packet.buff)) != sizeof(ack_packet.buff)) {
			return -1;
		}
	}

	return total_received;
}

int network_recv_file(int socket, uint8_t* file_name) {
	printf("%s Receive File\n", NETWORK_STR);

	packet_t packet;
	FILE* output;
	int bytes_received;
	bytes_received = 0;

	if ((output = fopen(file_name, "wb")) == NULL) {
		return -1;
	}

	packet.packet.eof = false;
	while (!packet.packet.eof) {
		if (network_recv_packet(socket, &packet) != sizeof(packet)) {
			return -1;
		}
		bytes_received += packet.packet.data_length;
		if (fwrite(packet.packet.data, 1, packet.packet.data_length, output) != packet.packet.data_length) {
			return bytes_received;
		}
	}
	if (fclose(output) < 0)
	{
		return -1;
	}
	return bytes_received;
}

int network_recv(int socket, uint8_t* buffer, unsigned int len) {
	printf("%s Receive\n", NETWORK_STR);

	packet_t packet;
	int bytes_received;

	if (network_recv_packet(socket, &packet) != sizeof(packet_t)) {
		return -1;
	}
	memcpy(buffer, packet.packet.data, bytes_received = packet.packet.data_length);
	return bytes_received;
}

/*
 * @brief Call physical layer connect on given url and port
 * @param url The server name to connect to
 * @param port The port that the server is listening on
 * @return Server socket that is connected
 */
int network_connect(char* url, unsigned short port)
{
	printf("%s Connect\n", NETWORK_STR);
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
	printf("%s Listen\n", NETWORK_STR);
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
	printf("%s Accept\n", NETWORK_STR);
	return physical_accept(socket, client_addr, client_len);
}
