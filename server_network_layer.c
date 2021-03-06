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
#include "data_link_layer.h"

/*
 * @brief Receive packets of file and write out to new photo file
 * @param socket The socketto receive from
 * @param file_name The file to output the photo to
 * @return bytes_received The bytes received in packets, or -1 on error
 * @author anivarthi
 */
int network_recv_file(int socket, char* file_name)
{
    packet_t packet;
    FILE* output;
    int bytes_received;
    bytes_received = 0;

    if ((output = fopen(file_name, "wb")) == NULL)
    {
        return -1;
    }

    packet.packet.eof = false;
    while (!packet.packet.eof)
    {
        if (data_link_recv_packet(socket, &packet) != sizeof(packet))
        {
            return -1;
        }

        if (data_link_send_ack_packet(socket) != 0)
        {
            return -1;
        }
        photo_log(socket, "ACK packet sent successfully.\n");

        bytes_received += packet.packet.data_length;
        if (fwrite(packet.packet.data, 1, packet.packet.data_length, output) != packet.packet.data_length)
        {
            return bytes_received;
        }
    }
    DEBUG(NETWORK_STR "Closing file\n");
    if (fclose(output) < 0)
    {
        DEBUG(NETWORK_STR "Close failed\n");
        return -1;
    }
    DEBUG(NETWORK_STR "File closed\n");
    return bytes_received;
}

/*
 * @brief Receive a data buffer of some length from the socket
 * @param socket The socket to receive the packet from
 * @param data The data to be filled
 * @param data_size The length of given data
 * @return data_size The number of bytes received, or -1 on error
 * @author anivarthi
 */
int network_recv(int socket, uint8_t* data, size_t data_size)
{
    int pos;
    unsigned int chunk_len;
    packet_t packet;

    for (pos = 0; pos < data_size; pos += chunk_len)
    {
        if (data_link_recv_packet(socket, &packet) != sizeof(packet_t))
        {
            return -1;
        }
        chunk_len = packet.packet.data_length;
        if (pos + chunk_len > data_size)
        {
            return -1;
        }
        memcpy(data + pos, packet.packet.data, chunk_len);

        if (data_link_send_ack_packet(socket) != 0)
        {
            return -1;
        }
        photo_log(socket, "ACK packet sent successfully.\n");
    }
    return data_size;
}
