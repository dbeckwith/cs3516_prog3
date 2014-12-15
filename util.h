#ifndef UTIL_H
#define UTIL_H

// Boolean type definition
#define bool int
#define false 0
#define true 1

// Logging
#define CLIENT_LOG_FILE "client.log"

// Function definitions

void exit_with_error(char* error);

int add_photo_log(int socket, char* file_name);
int close_photo_log(int socket);
int photo_log(int socket, const char* format, ...);

#endif
