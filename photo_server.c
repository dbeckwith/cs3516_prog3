#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXPENDING 5
#define SERVER_PORT 4167
#define ACK "Packet received"
#define DONE_CMD "DONE"
#define NEXT_CMD "NEXT FILE"
#define RCVBUFSIZE 256
#define QUIT_CMD "quit"
#define PHOTO_STR "photo"
#define NEW_STR "new"
#define PHOTO_EXT "jpg"

void exit_with_error(char *error);

void handle_client(int client_socket)
{
	char echoBuffer[RCVBUFSIZE];
	char photo_file_name[RCVBUFSIZE];
	FILE* output;
	int recvMsgSize;
	int client_id, photo_id;

	while (strcmp(echoBuffer, DONE_CMD) != 0)
	{
		if ((recvMsgSize = recv(client_socket, photo_file_name, RCVBUFSIZE, 0)) < 0)
		{
			exit_with_error("Recv() failed");
		}

		// int len = strlen(photo_file_name) - 1;
		// for (int i = len; i > len - 5; i--)
		// {
		// 	photo_file_name[i] = '\0';
		// }
		// strcat(photo_file_name, "new.jpg");

		sscanf(photo_file_name, PHOTO_STR "_%d_%d." PHOTO_EXT, &client_id, &photo_id);
		sprintf(photo_file_name, "%s%s_%d_%d.%s", PHOTO_STR, NEW_STR, client_id, photo_id, PHOTO_EXT);

		if ((output = fopen(photo_file_name, "w")) == NULL)
		{
			exit_with_error("File open");
			exit(1);
		}	

		while (strcmp(echoBuffer, NEXT_CMD) != 0 && strcmp(echoBuffer, DONE_CMD) != 0)
		{
			if (send(client_socket, ACK, strlen(ACK), 0) != strlen(ACK))
			{
				exit_with_error("Send() failed");
			}

			memset(echoBuffer, 0, RCVBUFSIZE);
			if ((recvMsgSize = recv(client_socket, echoBuffer, RCVBUFSIZE, 0)) < 0)
			{
				exit_with_error("Recv() failed");
			}
			//fprintf(output, "%s", echoBuffer);
			fwrite(echoBuffer, 1, RCVBUFSIZE, output);
		}
		fclose(output);

		if (send(client_socket, ACK, strlen(ACK), 0) != strlen(ACK))
		{
			exit_with_error("Send() failed");
		}
	}
	close(client_socket);
}

void *server_thread(void *arg)
{
	handle_client((long) arg);
}

int main(int argc, char *argv[])
{
	int serv_socket; // Server socket
	long client_socket; // Client socket
	struct sockaddr_in photo_serv_addr; // Local address
	struct sockaddr_in photo_client_addr; // Client address
	unsigned short echo_serv_port; // Port
	unsigned int client_len; // Length of client address data structure
	
	if (argc != 1)
	{ 
		fprintf(stderr, "Usage: %s\n", argv[0]);
		exit(1); 
	}
	echo_serv_port = SERVER_PORT;
	
	// Try socket for connection
	if ((serv_socket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
	{
		exit_with_error("Socket() failed");
	}
	
	memset(&photo_serv_addr, 0, sizeof(photo_serv_addr)); // Clear struct
	photo_serv_addr.sin_family = AF_INET; // Internet address family
	photo_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	photo_serv_addr.sin_port = htons(echo_serv_port); // Set port

	// Bind local address
	if (bind (serv_socket, (struct sockaddr *) &photo_serv_addr, sizeof(photo_serv_addr)) < 0)
	{
		exit_with_error("bind() failed");
	}
	
	// Listen on given port for clients
	if (listen (serv_socket, MAXPENDING) < 0)
	{
		exit_with_error("listen() failed");
	}
	while (1)
	{
		client_len = sizeof(photo_client_addr); // Length of client address

		if ((client_socket = accept(serv_socket, (struct sockaddr*) &photo_client_addr, &client_len)) < 0) 
		{
			exit_with_error("accept() failed");
		}
		printf("Handling client %s : %d\n", inet_ntoa(photo_client_addr.sin_addr), client_socket);
		
		pthread_t tid;
		if (pthread_create(&tid, NULL, server_thread, (void*)client_socket) != 0)
		{
			exit_with_error("Thread error");
		}
		pthread_detach(tid);
	}
}
