# Daniel Beckwith, Aditya Nivarthi
# Make file Project 3

CC=gcc
CFLAGS=

all: photo_client photo_server clean

photo_client: photo_client.o util.o
	$(CC) $(CFLAGS) -o team_3_photo_client photo_client.o util.o

photo_server: photo_server.o util.o
	$(CC) $(CFLAGS) -lpthread -o team_3_photo_server photo_server.o util.o

photo_client.o: photo_client.c photo.h util.h
	$(CC) $(CFLAGS) -c photo_client.c

photo_server.o: photo_server.c photo.h util.h
	$(CC) $(CFLAGS) -c photo_server.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

clean:
	rm -vf *.o
