#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netdb.h>
#include "photo.h"
#include "util.h"
#include "client_network_layer.h"
#include "network_layer.h"
#include "physical_layer.h"

#define CLIENT_STR "[PHOTO CLIENT]: "
#define SENDBUFSIZE 256
#define MAXFILENAME 256

int main(int argc, char* argv[])
{
    int sock;
    int photo_count;
    int photo_num;
    int client_id;
    char log_file_name[MAXFILENAME];
    uint8_t send_buff[SENDBUFSIZE];
    char photo_file_name[MAXFILENAME];
    size_t photo_file_name_len;

    // Bad arguments
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s <Server IP> <Client ID> <Photo Count> \n", argv[0]);
        exit(1);
    }

    if ((sock = physical_connect(argv[1], SERVER_PORT)) < 0)
    {
        exit_with_error("Network_connect() failed");
    }

    photo_count = atoi(argv[3]);
    client_id = atoi(argv[2]);

    sprintf(log_file_name, "client_%d.log", client_id);
    add_photo_log(sock, log_file_name);

    memcpy(send_buff, &client_id, sizeof(client_id));
    if (network_send(sock, send_buff, sizeof(client_id)) != sizeof(client_id))
    {
        exit_with_error("Network_send() sent a different number of bytes than expected for client id");
    }

    for (photo_num = 0; photo_num < photo_count; photo_num++)
    {
        photo_file_name_len = sprintf(photo_file_name, "%s_%d_%d.%s", PHOTO_STR, client_id, 1 + photo_num, PHOTO_EXT);
        if (DEBUG) printf(CLIENT_STR "%s\n", photo_file_name);

        memcpy(send_buff, &photo_file_name_len, sizeof(photo_file_name_len));
        if (network_send(sock, send_buff, sizeof(photo_file_name_len)) != sizeof(photo_file_name_len))
        {
            exit_with_error("Network_send() sent a different number of bytes than expected for file name length");
        }

        // Send photo name
        if (network_send(sock, photo_file_name, photo_file_name_len) != photo_file_name_len)
        {
            exit_with_error("Network_send() sent a different number of bytes than expected for file name");
        }

        // Send photo to server
        if (network_send_file(sock, photo_file_name) < 0) {
            exit_with_error("Network_send_file");
        }

        if (photo_num == photo_count - 1)
        {
            // Just sent last photo, tell server we're done
            send_buff[0] = DONE_CMD;
        }
        else
        {
            // Tell server to wait for another photo
            send_buff[0] = NEXT_CMD;
        }
        if (network_send(sock, send_buff, 1) != 1)
        {
            exit_with_error("Network_send() sent a different number of bytes than expected for command");
        }
    }

    if (DEBUG) printf(CLIENT_STR "Bye.\n");
    close_photo_log(sock);
    close(sock);
    exit(0);
}
