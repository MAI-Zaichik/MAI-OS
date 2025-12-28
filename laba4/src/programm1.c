#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../include/libs.h"

#define BUFFER_SIZE 4096

void command_1() {
    char* arg1 = strtok(NULL, " \t\n");
    char* arg2 = strtok(NULL, " \t\n");
    char* arg3 = strtok(NULL, " \t\n");
    
    int len = 0;
    char buffer[BUFFER_SIZE];

    if (arg1 && arg2 && arg3) {
        float a = atof(arg1);
        float b = atof(arg2);
        float e = atof(arg3);
        float res = sin_integral(a, b, e);
        len = snprintf(buffer, BUFFER_SIZE, "sin_integral(%.2f, %.2f, %.4f) result: %.6f\n", 
                      a, b, e, res);
        write(STDOUT_FILENO, buffer, len);
    } else {
        const char msg[] = "error: missing arguments for command 1 (need 3)\n";
        write(STDERR_FILENO, msg, sizeof(msg)-1);
    }
}

void command_2() {
    char* token;
    int array[100];
    size_t count = 0;
    
    while ((token = strtok(NULL, " \t\n")) != NULL && count < 100) {
        array[count++] = atoi(token);
    }
    
    int len = 0;
    char buffer[BUFFER_SIZE];

    if (count > 0) {
        int *res = sort(array, count);
        
        if (res) {
            len = snprintf(buffer, BUFFER_SIZE, "sort result: ");
            write(STDOUT_FILENO, buffer, len);
            
            for (size_t i = 0; i < count; ++i) {
                len = snprintf(buffer, BUFFER_SIZE, "%d ", res[i]);
                write(STDOUT_FILENO, buffer, len);
            }
            
            len = snprintf(buffer, BUFFER_SIZE, "\n");
            write(STDOUT_FILENO, buffer, len);
            
            free(res);
        } else {
            const char msg[] = "error: memory alloc\n";
            write(STDERR_FILENO, msg, sizeof(msg)-1);
        }
    } else {
        const char msg[] = "error: missing arguments for command 2 (need at least 1 number)\n";
        write(STDERR_FILENO, msg, sizeof(msg)-1);
    }
}

int main() {
    {
        const char* message = "static\ncommands: 1 - sin_integral a b e; 2 - sort numbers; ctrl d to exit\n> ";
        write(STDOUT_FILENO, message, strlen(message));
    }

    int bytes_read = 0;
    char buffer[BUFFER_SIZE];

    while((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = 0;

        char* token = strtok(buffer, " \t\n");
        if (!token) {
            write(STDOUT_FILENO, "> ", 2);
            continue;
        }

        int cmd = atoi(token);
        switch (cmd) {
            case 1:
                command_1();
                break;
            case 2:
                command_2();
                break;
            default:
                {
                   const char msg[] = "unknown command\n";
                   write(STDOUT_FILENO, msg, sizeof(msg)-1);
                }
                break;
        }

        write(STDOUT_FILENO, "> ", 2);
    }

    write(STDOUT_FILENO, "\n", 1);
    return 0;
}