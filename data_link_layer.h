
#ifndef DATA_LINK_LAYER_H
#define DATA_LINK_LAYER_H

enum frame_types {DATA, ACK};

typedef struct frame
{
	char frame_type;
	char[2] seq_num;
	char eof;
	char[2] error_detect;
} frame_t;

int data_link_send(int socket, char* buffer, int buffer_size);
int data_link_recv(int socket, char* buffer, int buffer_size);

#endif
