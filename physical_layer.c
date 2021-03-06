#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>
#include "util.h"
#include "photo.h"
#include "physical_layer.h"

/*
 * @brief Converts hostname and port number to IP address and connects socket
 * @param serverName Hostname to connect to
 * @param serverPort The port to connect to
 * @return serverSocket The socket that has been connected
 * @author djbeckwith
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

    return serverSocket;

}

/*
 * @brief Send given buffer to given socket
 * @param socket The socket to send to
 * @param buffer The buffer to send
 * @param buffer_size The size of the buffer to send
 * @return Result of send()
 * @author djbeckwith
 */
int physical_send_frame(int socket, frame_t* frame)
{
    DEBUG(PHYSICAL_STR "sending frame segment to actual network layer\n");
    physical_error(socket, frame);
    return send(socket, frame->bytes, sizeof(frame_t), 0);
}

/*
 * @brief This function is used to handle the timers on receive. Referenced from: http://developerweb.net/viewtopic.php?id=2933. See readme for more information on resources.
 */
/*
   Params:
      fd       -  (int) socket file descriptor
      buffer - (char*) buffer to hold data
      len     - (int) maximum number of bytes to recv()
      flags   - (int) flags (as the fourth param to recv() )
      to       - (int) timeout in milliseconds
   Results:
      int      - The same as recv, but -2 == TIMEOUT
   Notes:
      You can only use it on file descriptors that are sockets!
      'to' must be different to 0
      'buffer' must not be NULL and must point to enough memory to hold at least 'len' bytes
      I WILL mix the C and C++ commenting styles...
*/
int recv_to(int fd, char *buffer, int len, int flags, int to) {

    fd_set readset;
    int result, iof = -1;
    struct timeval tv;

    // Initialize the set
    FD_ZERO(&readset);
    FD_SET(fd, &readset);

    // Initialize time out struct
    tv.tv_sec = 0;
    tv.tv_usec = to * 1000;
    
    // Select()
    result = select(fd+1, &readset, NULL, NULL, &tv);

    // Check status
    if (result < 0)
    {
        return -1;
    }
    else if (result > 0 && FD_ISSET(fd, &readset))
    {
        // Set non-blocking mode
        if ((iof = fcntl(fd, F_GETFL, 0)) != -1)
        {
            fcntl(fd, F_SETFL, iof | O_NONBLOCK);
        }
        // receive
        result = recv(fd, buffer, len, flags);
        // set as before
        if (iof != -1)
        {
            fcntl(fd, F_SETFL, iof);
        }
        return result;
    }
    return -2;
}

/*
 * @brief Receive given buffer from given socket
 * @param socket The socket to receive from
 * @param frame The frame to receive into
 * @param timeout Defines whether to call timeout recv or normal recv
 * @return chunk_size on error, frame_size on success
 * @author anivarthi
 */
int physical_recv_frame(int socket, frame_t* frame, bool timeout)
{
    int pos;
    size_t frame_size;
    int chunk_size;

    frame_size = sizeof(frame_t);

    for (pos = 0; pos < frame_size; pos += chunk_size)
    {
        DEBUG(PHYSICAL_STR "Receiving frame segment from actual network layer\n");
        if (timeout)
        {
            if ((chunk_size = recv_to(socket, frame->bytes + pos, frame_size - pos, 0, TIMEOUT)) <= 0)
            {
                DEBUG(PHYSICAL_STR "Actual receive failed: %d\n", chunk_size);
                return chunk_size;
            }
        }
        else
        {
            if ((chunk_size = recv(socket, frame->bytes + pos, frame_size - pos, 0)) <= 0)
            {
                DEBUG(PHYSICAL_STR "Actual receive failed: %d\n", chunk_size);
                return chunk_size;
            }
        }
    }
    DEBUG(PHYSICAL_STR "Done receiving frame from actual network layer\n");

    return frame_size;
}

/*
 * @brief Listens on given port for a max number of clients
 * @param port The port to listen on
 * @param max_pending_clients The max number of clients to hold in connection queue
 * @return ser_socket The server socket being listened to
 * @author anivarthi
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
 * @author anivarthi
 */
int physical_accept(int socket, struct sockaddr_in* client_addr, unsigned int* client_len)
{
    int ret;
    ret = accept(socket, (struct sockaddr*)client_addr, client_len);
    printf(PHYSICAL_STR "Accepting client %s\n", inet_ntoa(client_addr->sin_addr));
    return ret;
}
