#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#include "network_layer.h"
#include "client_network_layer.h"
#include "data_link_layer.h"
#include "physical_layer.h"

/*
 * @brief Send packets from reading chunks of the given photo file
 * @param socket The socket to send the packet to
 * @param file_name The photo file to open and read
 * @return bytes_sent The number of bytes sent, or -1 on error
 * @author djbeckwith
 */
int network_send_file(int socket, char* file_name)
{
    // These variables are used to keep track of the current and previous buffers to check for EOF on packets
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

    packet_count = 0;

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
            // Copy file data into packet
            packet.packet.eof = *curr_read_size == 0;
            memcpy(packet.packet.data, prev_read_buffer, *prev_read_size);
            packet.packet.data_length = *prev_read_size;

            packet_count++;
            // Send packet through data link layer
            if (data_link_send_packet(socket, &packet) != sizeof(packet_t))
            {
                return -1;
            }

            photo_log(socket, "Packet %d sent successfully.\n", packet_count);

            bytes_sent += *prev_read_size;

            // Wait for an ACK that this packet got sent
            if (data_link_recv_ack_packet(socket) != 0) {
                return -1;
            }

            photo_log(socket, "Packet %d ACKed successfully.\n", packet_count);

            // If EOF is valid
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
 * @param data The buffer to be sent
 * @param data_size The length of given buffer
 * @return data_size The number of bytes sent, or -1 on error
 * @author djbeckwith
 */
int network_send(int socket, uint8_t* data, size_t data_size)
{
    packet_t packet;
    unsigned int pos;
    unsigned int chunk_len;

    packet_count = 0;

    chunk_len = PKT_DATA_SIZE;
    for (pos = 0; pos < data_size; pos += PKT_DATA_SIZE)
    {
        if (pos + PKT_DATA_SIZE >= data_size)
        {
            chunk_len = data_size - pos;
        }
        // Copy data into packet
        memcpy(packet.packet.data, data + pos, chunk_len);
        packet.packet.data_length = chunk_len;
        packet.packet.eof = false;
        
        packet_count++;

        // Send packet through data link layer
        DEBUG(NETWORK_STR "sending packet through data link layer\n");
        if (data_link_send_packet(socket, &packet) != sizeof(packet_t))
        {
            return -1;
        }

        photo_log(socket, "Packet %d sent successfully.\n", packet_count);

        // wWit for an ACK that this packet got sent
        DEBUG(NETWORK_STR "waiting for packet ack through data link layer\n");
        if (data_link_recv_ack_packet(socket) != 0) {
            return -1;
        }
        photo_log(socket, "Packet %d ACKed successfully.\n", packet_count);
    }
    return data_size;
}
