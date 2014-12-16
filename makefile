# Daniel Beckwith, Aditya Nivarthi
# Make file Project 3

CC=gcc
CFLAGS=

all: photo_client photo_server clean

photo_client: photo_client.o util.o client_network_layer.o client_data_link_layer.o data_link_layer.o physical_layer.o client_physical_layer_error.o
	$(CC) $(CFLAGS) -o team_3_photo_client photo_client.o util.o client_network_layer.o client_data_link_layer.o data_link_layer.o physical_layer.o client_physical_layer_error.o

photo_client.o: photo_client.c photo.h util.h
	$(CC) $(CFLAGS) -c photo_client.c

client_network_layer.o: client_network_layer.c client_network_layer.h network_layer.h
	$(CC) $(CFLAGS) -c client_network_layer.c

client_data_link_layer.o: client_data_link_layer.c client_data_link_layer.h data_link_layer.h
	$(CC) $(CFLAGS) -c client_data_link_layer.c

client_physical_layer_error.o: client_physical_layer_error.c physical_layer.h
	$(CC) $(CFLAGS) -c client_physical_layer_error.c

photo_server: photo_server.o util.o server_network_layer.o server_data_link_layer.o data_link_layer.o physical_layer.o server_physical_layer_error.o
	$(CC) $(CFLAGS) -lpthread -o team_3_photo_server photo_server.o util.o server_network_layer.o server_data_link_layer.o data_link_layer.o physical_layer.o server_physical_layer_error.o

photo_server.o: photo_server.c photo.h util.h
	$(CC) $(CFLAGS) -c photo_server.c

server_network_layer.o: server_network_layer.c server_network_layer.h network_layer.h
	$(CC) $(CFLAGS) -c server_network_layer.c

server_data_link_layer.o: server_data_link_layer.c server_data_link_layer.h data_link_layer.h
	$(CC) $(CFLAGS) -c server_data_link_layer.c

server_physical_layer_error.o: server_physical_layer_error.c physical_layer.h
	$(CC) $(CFLAGS) -c server_physical_layer_error.c

data_link_layer.o: data_link_layer.c data_link_layer.h
	$(CC) $(CFLAGS) -c data_link_layer.c

physical_layer.o: physical_layer.c physical_layer.h
	$(CC) $(CFLAGS) -c physical_layer.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

clean:
	rm -vf *.o
