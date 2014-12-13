#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "physical_layer.h"

int physical_connect(char *serverName, unsigned short serverPort) {
    int serverSocket;
    struct addrinfo serverAddrHints; // hints for finding server with DNS
    struct addrinfo *serverAddrInfo; // server address info
    struct sockaddr *serverAddr; // server address
    char serviceName[6]; // string version of "service" which is just the server port

    printf("Connecting to server at %s:%d\n", serverName, serverPort);

    sprintf(serviceName, "%d", serverPort); // convert port to string for service field

    memset(&serverAddrHints, 0, sizeof(struct addrinfo));
    serverAddrHints.ai_family = AF_INET;    // allow internet address family
    serverAddrHints.ai_socktype = SOCK_STREAM; // allow TCP type sockets

    // do a DNS lookup with the server name, the port number ("service"), and address hints
    if (getaddrinfo(serverName, serviceName, &serverAddrHints, &serverAddrInfo) != 0)
        exit_with_error("getaddrinfo() failed");

    // serverAddrInfo is a linked list of possible addresses, go through each and try to connect
    for (; serverAddrInfo != NULL; serverAddrInfo = serverAddrInfo->ai_next) {
        // create a new socket from the info
        if ((serverSocket = socket(serverAddrInfo->ai_family, serverAddrInfo->ai_socktype, serverAddrInfo->ai_protocol)) < 0) {
            continue;
        }

        // get address
        serverAddr = serverAddrInfo->ai_addr;

        // try to connect
        if (connect(serverSocket, serverAddr, sizeof(*serverAddr)) < 0) {
            // didn't connect, close the socket and keep going
            close(serverSocket);
            continue;
        }
        // connected, break the loop
        break;
    }

    // if got through the whole list and never connected, print an error
    if (serverAddrInfo == NULL) {
        return -1;
    }

    printf("Connected to server at %s\n", inet_ntoa(((struct sockaddr_in *)serverAddr)->sin_addr));

    return serverSocket;

}

int physical_send(int socket, char* buffer, int buffer_size)
{
    return send(socket, buffer, buffer_size, 0);
}

int physical_recv(int socket, char* buffer, int buffer_size)
{
    return recv(socket, buffer, buffer_size, 0);
}

int physical_listen(unsigned short port, unsigned int max_pending_clients)
{
    int serv_socket;
    struct sockaddr_in photo_serv_addr; // Local address

    if ((serv_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
    {
        return -1;
    }
    
    memset(&photo_serv_addr, 0, sizeof(photo_serv_addr)); // Clear struct
    photo_serv_addr.sin_family = AF_INET; // Internet address family
    photo_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    photo_serv_addr.sin_port = htons(port); // Set port

    // Bind local address
    if (bind (serv_socket, (struct sockaddr*)&photo_serv_addr, sizeof(photo_serv_addr)) < 0)
    {
        return -1;
    }
    
    // Listen on given port for clients
    if (listen (serv_socket, max_pending_clients) < 0)
    {
        return -1;
    }
    return serv_socket;
}

int physical_accept(int socketfd, struct sockaddr* client_addr, unsigned int* client_len)
{
    return accept(socketfd, client_addr, client_len);
}
