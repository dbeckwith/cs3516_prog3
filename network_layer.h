#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

int network_send(int sockfd, char* buffer, unsigned int len);
int network_recv(int sockfd, char* buffer, unsigned int len);

int network_connect(char* url, unsigned short port);

int network_listen(unsigned short port, unsigned int max_pending_clients);
int network_accept(int sockfd);

#endif
