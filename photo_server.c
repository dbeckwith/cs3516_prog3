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
#include "network_layer.h"

#define MAXPENDING 5 //   Max number of clients in queue
#define RCVBUFSIZE 256
#define SERVER_STR "[PHOTO SERVER]: "

int recv_message(int socket, char* buffer, unsigned int len);
void handle_client(int client_socket);

/*
 * @brief Function for thread to run when a new client is accepted. Handles client by receiving packets of photos
 * @param *arg The client socket to handle
 */
void *server_thread(void *arg)
{
	handle_client((long) arg);
}

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

	if ((serv_socket = network_listen(SERVER_PORT, MAXPENDING)) < 0)
	{
		exit_with_error("Network_listen() failed");
	}

	// Server runs continuously
	while (true)
	{
		client_addr_len = sizeof(photo_client_addr); // Length of client address

		if ((client_socket = network_accept(serv_socket, (struct sockaddr*)&photo_client_addr, &client_addr_len)) < 0) 
		{
			exit_with_error("Network_accept() failed");
		}

		printf(SERVER_STR "Handling client %s : %d\n", inet_ntoa(photo_client_addr.sin_addr), client_socket);
		
		pthread_t tid;
		if (pthread_create(&tid, NULL, server_thread, (void*)client_socket) != 0)
		{
			exit_with_error("Thread error");
		}
		pthread_detach(tid);
	}
}


/*
 * @brief Handles client given a socket. Receives photo name and calls protocols in lower levels to handle receiving packets from client.
 * @param client_socket The socket to receive the photo(s) from
 */
void handle_client(int client_socket)
{
	uint8_t recv_buff[RCVBUFSIZE];
	unsigned int photo_file_len;
	char* photo_file_name; //  Buffer for output photo file name
	int command;
	int client_id;
	int photo_id;

	command = -1;
	while (command != DONE_CMD)
	{
		if (recv_message(client_socket, recv_buff, 4) != 4)
		{
			exit_with_error("Recv() failed");
		}

		memcpy(&client_id, recv_buff, 4);

		if (recv_message(client_socket, recv_buff, 4) != 4)
		{
			exit_with_error("Recv() failed");
		}

		memcpy(&photo_file_len, recv_buff, 4);

		if (recv_message(client_socket, recv_buff, photo_file_len) != photo_file_len)
		{
			exit_with_error("Recv() failed");
		}
		photo_file_name = (char*)malloc(photo_file_len);
		memcpy(photo_file_name, recv_buff, photo_file_len);

		// Format the name to the new file name
		sscanf(photo_file_name, PHOTO_STR "_%d_%d." PHOTO_EXT, &client_id, &photo_id);
		sprintf(photo_file_name, "%s%s_%d_%d.%s", PHOTO_STR, NEW_STR, client_id, photo_id, PHOTO_EXT);

		// While not DONE or NEXT FILE, keep receving photo packets
		if (network_recv_file(client_socket, photo_file_name) < 0)
		{
			exit_with_error("Network_recv() failed");
		}

		if (recv_message(client_socket, recv_buff, 1) != 1)
		{
			exit_with_error("Recv() failed");
		}
		command = recv_buff[0];
	}
	close(client_socket);
}

/*
Receives exactly len bytes from the client.
*/
int recv_message(int socket, char* buffer, unsigned int len)
{
	unsigned int pos;
	int chunk_len;

	for (pos = 0; pos < len; pos += chunk_len)
	{
		chunk_len = network_recv(socket, buffer + pos, len - pos);
		if (chunk_len <= 0)
		{
			return pos;
		}
	}
	return pos;
}
