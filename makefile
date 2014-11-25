CC=gcc
CLIENT_NAME=client
SERVER_NAME=server

client: client.o
	$(CC) client.o -c $(CLIENT_NAME)



server: server.o
	$(CC) server.o -c $(SERVER_NAME)



clean:
	rm -vf *.o $(CLIENT_NAME) $(SERVER_NAME)
