# Daniel Beckwith, Aditya Nivarthi
# Make file Project 3

CC=gcc

CFLAGS= -g -std=c99 -lpthread

all: photo_client photo_server clean

photo_client: photo_client.o exit_with_error.o
	$(CC) $(CFLAGS) -o team_3_photo_client photo_client.o exit_with_error.o

photo_server: photo_server.o exit_with_error.o
	$(CC) $(CFLAGS) -o team_3_photo_server photo_server.o exit_with_error.o

photo_client.o: photo_client.c
	$(CC) $(CFLAGS) -c photo_client.c

photo_server.o: photo_server.c exit_with_error.c
	$(CC) $(CFLAGS) -c photo_server.c

exit_with_error.o: exit_with_error.c
	$(CC) $(CFLAGS) -c exit_with_error.c

clean:
	rm -v *.o