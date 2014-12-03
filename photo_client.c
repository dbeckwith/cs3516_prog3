#define _GNU_SOURCE

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <features.h>

#define SERVER_PORT "4167"
#define RCVBUFSIZE 256
#define QUIT_CMD "quit\n"

void exit_with_error(char* error);

/*
 * @brief Converts hostname and port number to IP address and connects socket
 * @param address The hostname to connect to
 * @param server_port The port to connect to
 * @return socket_conn The socket descriptor that has been connected
 */
int host_to_IP_connection(char* address, char* server_port)
{
	int socket_conn, addr_result;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // Datagram socket
	hints.ai_flags = 0;
	hints.ai_protocol = 0; // Any protocol

	addr_result = getaddrinfo(address, server_port, &hints, &result); // Get address from hostname

	if (addr_result != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addr_result));
		exit(EXIT_FAILURE);
	}

	// Search through list of results for socket connection that works correctly
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		socket_conn = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (socket_conn == -1)
		{
			continue; // Don't try the connection since socket failed
		}

		if (connect(socket_conn, rp->ai_addr, rp->ai_addrlen) != -1)
		{
			break;
		}
		close(socket_conn);
	}

	freeaddrinfo(result); // Free memory of iterator variable
	return socket_conn;
}

int main(int argc, char** argv)
{
	int sock, quit = 0;
	size_t buflen = 0;
	char* input_string;
	char output_string[RCVBUFSIZE];
	unsigned int input_len;
	int bytes_rcvd, total_bytes_rcvd;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <Server IP>\n", argv[0]);
		exit(1);
	}

	if ((sock = host_to_IP_connection(argv[1], SERVER_PORT)) < 0)
	{
		exit_with_error(" Connect() failed");
	}

	while (!quit)
	{
		printf("--->\n");
		if ((input_len = getline(&input_string, &buflen, stdin)) < 0)
		{
			exit_with_error("Getline failed\n");
		}

		if (send(sock, input_string, input_len, 0) != input_len)
		{
			exit_with_error("send() sent a different number of bytes than expected");
		}
		
		total_bytes_rcvd = 0;

		while (total_bytes_rcvd < input_len)
		{
			if ((bytes_rcvd = recv(sock, output_string, RCVBUFSIZE - 1, 0)) <= 0)
			{
				exit_with_error("recv() failed or connection closed prematurely");
			}

			total_bytes_rcvd += bytes_rcvd;
			output_string[bytes_rcvd] = '\0';
			printf("Output: %s", output_string);
		}

		if (strcmp(input_string, QUIT_CMD) == 0)
		{
			quit = 1;
		}
	}

	close(sock);
	exit(0);
}
