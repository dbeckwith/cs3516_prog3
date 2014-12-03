#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXPENDING 5
#define SERVER_PORT 4167
#define RCVBUFSIZE 256
#define QUIT_CMD "quit"

void exit_with_error(char *error);

void handle_client(int client_socket)
{
	char echoBuffer[RCVBUFSIZE];
	int recvMsgSize;

	if ((recvMsgSize = recv(client_socket, echoBuffer, RCVBUFSIZE, 0)) < 0)
	{
		exit_with_error("Recv() failed");
	}

	while (recvMsgSize > 0)
	{
		if (send(client_socket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
		{
			exit_with_error("Send() failed");
		}

		if ((recvMsgSize = recv(client_socket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		{
			exit_with_error("Recv() failed");
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
	struct sockaddr_in echo_serv_addr; // Local address
	struct sockaddr_in echo_client_addr; // Client address
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
	
	memset(&echo_serv_addr, 0, sizeof(echo_serv_addr)); // Clear struct
	echo_serv_addr.sin_family = AF_INET; // Internet address family
	echo_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	echo_serv_addr.sin_port = htons(echo_serv_port); // Set port

	// Bind local address
	if (bind (serv_socket, (struct sockaddr *) &echo_serv_addr, sizeof(echo_serv_addr)) < 0)
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
		client_len = sizeof(echo_client_addr); // Length of client address

		if ((client_socket = accept(serv_socket, (struct sockaddr*) &echo_client_addr, &client_len)) < 0) 
		{
			exit_with_error("accept() failed");
		}
		printf("Handling client %s : %d\n", inet_ntoa(echo_client_addr.sin_addr), client_socket);
		
		pthread_t tid;
		if (pthread_create(&tid, NULL, server_thread, (void*)client_socket) != 0)
		{
			exit_with_error("Thread error");
		}
		pthread_detach(tid);
	}
}