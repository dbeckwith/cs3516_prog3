#ifndef PHYSICAL_LAYER_H
#define PHYSICAL_LAYER_H

#define PHYSICAL_STR "PHYSICAL LAYER" // String to display for debugging at physical layer

// Function definitions

int physical_connect(char *serverName, unsigned short serverPort);
int physical_send(int socket, char* buffer, int buffer_size);
int physical_recv(int socket, char* buffer, int buffer_size);
int physical_listen(unsigned short port, unsigned int max_pending_clients);
int physical_accept(int socket, struct sockaddr* client_addr, unsigned int* client_len);

#endif
