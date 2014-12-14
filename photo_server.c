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
#define RCVBUFSIZE 256 // TODO: Remove this
#define ACK "ack" //      TODO: Remove this


/*
 * @brief Handles client given a socket. Receives photo name and calls protocols in lower levels to handle receiving packets from client.
 * @param client_socket The socket to receive the photo(s) from
 */
void handle_client(int client_socket)
{
	char input_buffer[RCVBUFSIZE]; //        Buffer for client data
	char photo_file_name[RCVBUFSIZE]; //     Buffer for output photo file name
	FILE* output; //                         Output file
	int recvMsgSize, client_id, photo_id;

	while (strcmp(input_buffer, DONE_CMD) != 0)
	{
		// Get the file name that will be the new photo
		if ((recvMsgSize = recv(client_socket, photo_file_name, RCVBUFSIZE, 0)) < 0)
		{
			exit_with_error("Recv() failed");
		}

		// Format the name to the new file name
		sscanf(photo_file_name, PHOTO_STR "_%d_%d." PHOTO_EXT, &client_id, &photo_id);
		sprintf(photo_file_name, "%s%s_%d_%d.%s", PHOTO_STR, NEW_STR, client_id, photo_id, PHOTO_EXT);

		if ((output = fopen(photo_file_name, "wb")) == NULL)
		{
			exit_with_error("File open");
			exit(1);
		}	

		// While not DONE or NEXT FILE, keep receving photo packets
		while (strcmp(input_buffer, NEXT_CMD) != 0 && strcmp(input_buffer, DONE_CMD) != 0)
		{
			if (send(client_socket, ACK, strlen(ACK), 0) != strlen(ACK))
			{
				exit_with_error("Send() failed");
			}

			memset(input_buffer, 0, RCVBUFSIZE);
			if ((recvMsgSize = recv(client_socket, input_buffer, RCVBUFSIZE, 0)) < 0)
			{
				exit_with_error("Recv() failed");
			}
			fwrite(input_buffer, 1, RCVBUFSIZE, output);
		}
		fclose(output);

		if (send(client_socket, ACK, strlen(ACK), 0) != strlen(ACK))
		{
			exit_with_error("Send() failed");
		}
	}
	close(client_socket);
}

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
		exit_with_error("listen() failed");
	}

	// Server runs continuously
	while (true)
	{
		client_addr_len = sizeof(photo_client_addr); // Length of client address

		if ((client_socket = network_accept(serv_socket, (struct sockaddr*) &photo_client_addr, &client_addr_len)) < 0) 
		{
			exit_with_error("accept() failed");
		}

		printf("Handling client %s : %d\n", inet_ntoa(photo_client_addr.sin_addr), client_socket);
		
		pthread_t tid;
		if (pthread_create(&tid, NULL, server_thread, (void*)client_socket) != 0)
		{
			exit_with_error("Thread error");
		}
		pthread_detach(tid); // TODO: Do we still want this?
	}
}
