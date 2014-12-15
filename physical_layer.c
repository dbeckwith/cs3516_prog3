#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#include "photo.h"
#include "physical_layer.h"

void set_phys_timeout()
{
    PHYS_TIMEOUT.tv_sec = 1;
    PHYS_TIMEOUT.tv_usec = 0;
}

/*
 * @brief Converts hostname and port number to IP address and connects socket
 * @param serverName Hostname to connect to
 * @param serverPort The port to connect to
 * @return serverSocket The socket that has been connected
 */
int physical_connect(char *serverName, unsigned short serverPort)
{
    int serverSocket;
    struct addrinfo serverAddrHints; // Hints for finding server with DNS
    struct addrinfo *serverAddrInfo; // Server address info
    struct sockaddr *serverAddr; //     Server address
    char serviceName[6]; //             String version of "service" which is just the server port

    printf(PHYSICAL_STR "Connecting to server at %s:%d\n", serverName, serverPort);

    sprintf(serviceName, "%d", serverPort); // Convert port to string for service field

    memset(&serverAddrHints, 0, sizeof(struct addrinfo));
    serverAddrHints.ai_family = AF_INET;    //    Allow internet address family
    serverAddrHints.ai_socktype = SOCK_STREAM; // Allow TCP type sockets

    // Do a DNS lookup with the server name, the port number ("service"), and address hints
    if (getaddrinfo(serverName, serviceName, &serverAddrHints, &serverAddrInfo) != 0)
    {
        exit_with_error("Getaddrinfo() failed");
    }

    // ServerAddrInfo is a linked list of possible addresses, go through each and try to connect
    for (; serverAddrInfo != NULL; serverAddrInfo = serverAddrInfo->ai_next)
    {
        // Create a new socket from the info
        if ((serverSocket = socket(serverAddrInfo->ai_family, serverAddrInfo->ai_socktype, serverAddrInfo->ai_protocol)) < 0)
        {
            continue;
        }

        // Get address
        serverAddr = serverAddrInfo->ai_addr;

        // Try to connect
        if (connect(serverSocket, serverAddr, sizeof(*serverAddr)) < 0)
        {
            // Didn't connect, close the socket and keep going
            close(serverSocket);
            continue;
        }
        // Connected, break the loop
        break;
    }

    // If got through the whole list and never connected, print an error
    if (serverAddrInfo == NULL)
    {
        return -1;
    }

    printf(PHYSICAL_STR "Connected to server at %s\n", inet_ntoa(((struct sockaddr_in *)serverAddr)->sin_addr));

    set_phys_timeout();

    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &PHYS_TIMEOUT, sizeof(struct timeval));
    return serverSocket;

}

/*
 * @brief Send given buffer to given socket
 * @param socket The socket to send to
 * @param buffer The buffer to send
 * @param buffer_size The size of the buffer to send
 * @return Result of send()
 */
int physical_send_frame(int socket, frame_t* frame)
{
    physical_error(frame);
    return send(socket, frame->bytes, sizeof(frame_t), 0);
}

/*
 * @brief Receive given buffer from given socket
 * @param socket The socket to receive from
 * @param buffer The buffer to receive into
 * @param buffer_size The size of the buffer to receive into
 * @return Result of recv()
 */
int physical_recv_frame(int socket, frame_t* frame)
{
    return recv(socket, frame->bytes, sizeof(frame_t), 0);
}

/*
 * @brief Listens on given port for a max number of clients
 * @param port The port to listen on
 * @param max_pending_clients The max number of clients to hold in connection queue
 * @return ser_socket The server socket being listened to
 */
int physical_listen(unsigned short port, unsigned int max_pending_clients)
{
    int serv_socket;
    struct sockaddr_in photo_serv_addr; // Local address

    if ((serv_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
    {
        return -1;
    }
    
    memset(&photo_serv_addr, 0, sizeof(photo_serv_addr)); // Clear struct
    photo_serv_addr.sin_family = AF_INET; //                 Internet address family
    photo_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //  Any incoming interface
    photo_serv_addr.sin_port = htons(port); //               Set port

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

/*
 * @brief Call accept on new client
 * @param socket The socket descriptor to accept
 * @param client_addr The client address struct to accept
 * @param client_len The length of the client address being given
 * @return The client socket that is accepted
 */
int physical_accept(int socket, struct sockaddr* client_addr, unsigned int* client_len)
{
    return accept(socket, client_addr, client_len);
}
