#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "network_layer.h"
#include "data_link_layer.h"
#include "physical_layer.h"


int network_send(int sockfd, char* buffer, unsigned int len)
{

}

int network_recv(int sockfd, char* buffer, unsigned int len)
{

}

int network_connect(char* url, unsigned short port)
{
	return connect_dns(url, port);
}

int network_listen(unsigned short port, unsigned int max_pending_clients)
{
	return physical_listen(port, max_pending_clients);
}

int network_accept(int socketfd, struct sockaddr* client_addr, unsigned int* client_len)
{
	return physical_accept(socketfd, client_addr, client_len);
}