
#ifndef DATA_LINK_LAYER_H
#define DATA_LINK_LAYER_H

#define FRAME_DATA_SIZE 124

enum frame_types {DATA, ACK};

typedef union {
    struct frame {
    	char frame_type;
    	char seq_num[2];
    	char data[FRAME_DATA_SIZE];
    	char data_length;
    	char eof;
    	char error_detect[2];
    } frame;
    char buff[sizeof(struct frame)];
} frame_t;

int data_link_send(int socket, char* buffer, int buffer_size);
int data_link_recv(int socket, char* buffer, int buffer_size);

#endif
