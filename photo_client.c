#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "photo.h"
#include "util.h"
#include "network_layer.h"
#include "data_link_layer.h"
#include "physical_layer.h"

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
	int sock, quit = 0, photo_count = 0, client_id = -1, photo_num = 0;
	FILE* fdIn;
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

	if ((sock = connect_dns(argv[1], SERVER_PORT)) < 0)
	{
		exit_with_error(" Connect() failed");
	}

	photo_count = atoi(argv[3]);
	client_id = atoi(argv[2]);

	for (photo_num = 0; photo_num < photo_count; photo_num++)
	{
		photo_file_name_len = sprintf(photo_file_name, "%s_%d_%d.%s", PHOTO_STR, client_id, 1 + photo_num, PHOTO_EXT);
		printf("%s\n", photo_file_name);

		if ((fdIn = fopen(photo_file_name, "rb")) < 0)
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

		while ((read_size = fread(photo_read_buffer, 1, RCVBUFSIZE, fdIn)) > 0)
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

		if (photo_num == photo_count - 1)
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
