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

#define SENDBUFLEN 256

int main(int argc, char* argv[])
{
	int sock;
    int photo_count;
    int photo_num;
    int client_id;
    char send_buff[SENDBUFLEN];
	char photo_file_name[1000];
	unsigned int photo_file_name_len;

	// Bad arguments
	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s <Server IP> <Client ID> <Photo Count> \n", argv[0]);
		exit(1);
	}

	if ((sock = network_connect(argv[1], SERVER_PORT)) < 0)
	{
		exit_with_error(" Connect() failed");
	}

	photo_count = atoi(argv[3]);
	client_id = atoi(argv[2]);

	for (photo_num = 0; photo_num < photo_count; photo_num++)
	{
		photo_file_name_len = sprintf(photo_file_name, "%s_%d_%d.%s", PHOTO_STR, client_id, 1 + photo_num, PHOTO_EXT);
		printf("%s\n", photo_file_name);

		printf("sending photo length %d\n", photo_file_name_len);
		memcpy(send_buff, &photo_file_name_len, 4);
		if (network_send(sock, send_buff, 4) != 4)
		{
			exit_with_error("send() sent a different number of bytes than expected");
		}

		printf("sending photo name\n");
        // Send photo name
		if (network_send(sock, photo_file_name, photo_file_name_len) != photo_file_name_len)
		{
			exit_with_error("send() sent a different number of bytes than expected");
		}

        // Send photo to server
        if (network_send_file(sock, photo_file_name) < 0) {
            exit_with_error("send file");
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
			exit_with_error("send() sent a different number of bytes than expected");
		}
	}

	printf("Bye.\n");

	close(sock);
	exit(0);
}
