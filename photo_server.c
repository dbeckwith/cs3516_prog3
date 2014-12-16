#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include "photo.h"
#include "util.h"
#include "server_network_layer.h"
#include "network_layer.h"
#include "physical_layer.h"

#define MAXPENDING 5 //   Max number of clients in queue
#define RCVBUFSIZE 256
#define SERVER_STR "[PHOTO SERVER]: "

void handle_client(int client_socket);

/*
 * @brief Main server function. Forks children when accepting new connections
 * @author anivarthi
 */
int main(int argc, char *argv[])
{
    int serv_socket; //                      Server socket
    long client_socket; //                   Client socket
    struct sockaddr_in photo_client_addr; // Client address
    unsigned int client_addr_len; //         Length of client address data structure
    
    if (argc != 1)
    { 
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(1); 
    }

    if ((serv_socket = physical_listen(SERVER_PORT, MAXPENDING)) < 0)
    {
        exit_with_error("Listen failed");
    }

    // Server runs continuously
    while (true)
    {
        client_addr_len = sizeof(photo_client_addr); // Length of client address

        if ((client_socket = physical_accept(serv_socket, &photo_client_addr, &client_addr_len)) < 0) 
        {
            exit_with_error("Accept failed");
        }

        pid_t pid;
        if ((pid = fork()) < 0) {
            exit_with_error("Fork error");
        }
        else if (pid == 0) {
            handle_client(client_socket);
            exit(0);
        }
    }
}


/*
 * @brief Handles client given a socket. Receives photo name and calls protocols in lower levels to handle receiving packets from client.
 * @param client_socket The socket to receive the photo(s) from
 * @author djbeckwith
 */
void handle_client(int client_socket)
{
    uint8_t recv_buff[RCVBUFSIZE];
    size_t photo_file_name_len;
    char* photo_file_name; //  Buffer for output photo file name
    int command;
    int client_id;
    int photo_id;
    char log_file_name[RCVBUFSIZE];

    if (network_recv(client_socket, recv_buff, sizeof(client_id)) != sizeof(client_id))
    {
        exit_with_error("Receive got a different number of bytes than expected for client id");
    }

    memcpy(&client_id, recv_buff, sizeof(client_id));

    printf(SERVER_STR "Handling client %d\n", client_id);

    sprintf(log_file_name, "server_%d.log", client_id);
    add_photo_log(client_socket, log_file_name);

    command = -1;
    while (command != DONE_CMD)
    {

        if (network_recv(client_socket, recv_buff, sizeof(photo_file_name_len)) != sizeof(photo_file_name_len))
        {
            exit_with_error("Receive got a different number of bytes than expected for file name length");
        }

        memcpy(&photo_file_name_len, recv_buff, sizeof(photo_file_name_len));

        if (network_recv(client_socket, recv_buff, photo_file_name_len) != photo_file_name_len)
        {
            exit_with_error("Receive got a different number of bytes than expected for file name");
        }
        photo_file_name = (char*)malloc(photo_file_name_len);
        memcpy(photo_file_name, recv_buff, photo_file_name_len);

        // Format the name to the new file name
        sscanf(photo_file_name, PHOTO_STR "%1d%1d." PHOTO_EXT, &client_id, &photo_id);
        sprintf(photo_file_name, "%s%s%d%d.%s", PHOTO_STR, NEW_STR, client_id, photo_id, PHOTO_EXT);

        printf(SERVER_STR "[Client %d]: Receiving photo #%d\n", client_id, photo_id);

        // While not DONE or NEXT FILE, keep receiving photo packets
        if (network_recv_file(client_socket, photo_file_name) < 0)
        {
            exit_with_error("Receive file failed");
        }

        printf(SERVER_STR "[Client %d]: Photo saved to %s\n", client_id, photo_file_name);

        if (network_recv(client_socket, recv_buff, 1) != 1)
        {
            exit_with_error("Receive got a different number of bytes than expected for command");
        }
        command = recv_buff[0];
    }
    close_photo_log(client_socket);
    close(client_socket);
}
