#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "physical_layer.h"

int connect_dns(char *serverName, unsigned short serverPort) {
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
