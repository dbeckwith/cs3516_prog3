#include <stdio.h>
#include <stdlib.h>
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
