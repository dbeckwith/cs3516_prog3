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
#define ACK "Packet received"
#define DONE_CMD "DONE"
#define NEXT_CMD "NEXT FILE"
#define RCVBUFSIZE 256
#define QUIT_CMD "quit\n"
#define PHOTO_STR "photo"
#define NEW_STR "new"
#define PHOTO_EXT "jpg"

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

int validate_ack(char* ack)
{
	if (strcmp(ack, ACK) != 0)
	{
		exit_with_error("No ack received");
		return 0;
	}
	return 1;
}

int main(int argc, char** argv)
{
	int sock, quit = 0, photo_count = 0, fdIn = -1, client_id = -1;
	size_t buflen = 0;
	char photo_file_name[RCVBUFSIZE];
	char ack_string[RCVBUFSIZE];
	unsigned int photo_file_name_len;
	int bytes_rcvd, total_bytes_rcvd;

	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s <Server IP> <Client ID> <Photo Count> \n", argv[0]);
		exit(1);
	}

	if ((sock = host_to_IP_connection(argv[1], SERVER_PORT)) < 0)
	{
		exit_with_error(" Connect() failed");
	}

	photo_count = atoi(argv[3]);
	client_id = atoi(argv[2]);

	for (int i = 0; i < photo_count; i++)
	{
		// if ((photo_file_name_len = getline(&photo_file_name, &buflen, stdin)) < 0)
		// {
		// 	exit_with_error("Getline failed\n");
		// }

		// char* nl = strrchr(photo_file_name, '\n'); ///TODO genreate photo file name
		// if (nl)
		// {
		// 	*nl = '\0';
		// 	photo_file_name_len--;
		// }

		photo_file_name_len = sprintf(photo_file_name, "%s_%d_%d.%s", PHOTO_STR, client_id, 1 + i, PHOTO_EXT);
		printf("%s\n", photo_file_name);

		if ((fdIn = open(photo_file_name, O_RDONLY)) < 0)
		{
			exit_with_error("File open");
			exit(1);
		}

		int read_size = 0;
		char photo_read_buffer[RCVBUFSIZE];

		if (send(sock, photo_file_name, photo_file_name_len, 0) != photo_file_name_len)
		{
			exit_with_error("send() sent a different number of bytes than expected");
		}

		if ((bytes_rcvd = recv(sock, ack_string, RCVBUFSIZE - 1, 0)) <= 0)
		{
			exit_with_error("recv() failed or connection closed prematurely");
		}

		validate_ack(ack_string);

		while ((read_size = read(fdIn, photo_read_buffer, RCVBUFSIZE)) > 0)
		{
			if (send(sock, photo_read_buffer, read_size, 0) != read_size)
			{
				exit_with_error("send() sent a different number of bytes than expected");
			}

			if ((bytes_rcvd = recv(sock, ack_string, RCVBUFSIZE - 1, 0)) <= 0)
			{
				exit_with_error("recv() failed or connection closed prematurely");
			}

			ack_string[bytes_rcvd] = '\0';
			validate_ack(ack_string);
		}

		if (i == photo_count - 1)
		{
			if (send(sock, DONE_CMD, strlen(DONE_CMD), 0) != strlen(DONE_CMD))
			{
				exit_with_error("send() sent a different number of bytes than expected");
			}
		}
		else
		{
			if (send(sock, NEXT_CMD, strlen(NEXT_CMD), 0) != strlen(NEXT_CMD))
			{
				exit_with_error("send() sent a different number of bytes than expected");
			}
		}
		if ((bytes_rcvd = recv(sock, ack_string, RCVBUFSIZE - 1, 0)) <= 0)
		{
			exit_with_error("recv() failed or connection closed prematurely");
		}
		validate_ack(ack_string);
	}

	printf("Bye.\n");

	close(sock);
	exit(0);
}
