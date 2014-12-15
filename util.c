#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "util.h"

/*
 * @brief Creates error and includes given message as output.
 * @param error The error message to output in addition to perror.
 */
void exit_with_error(char* error)
{
    perror(error);
    exit(1);
}

typedef struct log_map_entry {
    int socket;
    FILE* log_file;
    struct log_map_entry* next;
} log_map_entry_t;
log_map_entry_t* log_map;

int add_photo_log(int socket, char* file_name) {
    log_map_entry_t* new_entry;
    new_entry = (log_map_entry_t*)malloc(sizeof(log_map_entry_t));
    new_entry->socket = socket;
    new_entry->next = NULL;

    if ((new_entry->log_file = fopen(file_name, "w")) == NULL) {
        return -1;
    }

    if (log_map == NULL) {
        log_map = new_entry;
    }
    else {
        log_map_entry_t* curr_entry;
        for (curr_entry = log_map; curr_entry->next != NULL; curr_entry = curr_entry->next);
        curr_entry->next = new_entry;
    }
    return 0;
}

int close_photo_log(int socket) {
    log_map_entry_t* curr_entry;
    if (log_map == NULL) {
        return -1;
    }
    if (log_map->socket == socket) {
        if (fclose(log_map->log_file) != 0) {
            return -1;
        }
        log_map = log_map->next;
        return 0;
    }
    for (curr_entry = log_map; curr_entry->next != NULL; curr_entry = curr_entry->next) {
        if (curr_entry->next->socket == socket) {
            if (fclose(curr_entry->next->log_file) != 0) {
                return -1;
            }
            curr_entry->next = curr_entry->next->next;
            return 0;
        }
    }
    return -1;
}


int photo_log(int socket, const char* format, ...) {
    log_map_entry_t* curr_entry;
    for (curr_entry = log_map; curr_entry != NULL; curr_entry = curr_entry->next) {
        if (curr_entry->socket == socket) {
            va_list argptr;
            va_start(argptr, format);
            vfprintf(curr_entry->log_file, format, argptr);
            va_end(argptr);
            fflush(curr_entry->log_file);
            return 0;
        }
    }
    return -1;
}
