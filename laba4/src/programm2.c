#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "../include/libs.h"

#define BUFFER_SIZE 4096

typedef float (*sin_integral_func_t)(float, float, float);
typedef int* (*sort_func_t)(int*, size_t);

enum ErrorCode {
    OK = 0,
    ER_DLOPEN = -1,
    ER_DLSYM = -2,
};

enum CurrentLib {
    FIRST = 0,
    SECOND = 1,
};

enum ErrorCode command_0(const char** LIB_NAMES, void** library, int* current_lib, 
                         sin_integral_func_t* sin_integral_func, sort_func_t* sort_func) {
    char buffer[BUFFER_SIZE];
    
    if (*library) {
        dlclose(*library);
        *library = NULL;
    }

    *current_lib = (*current_lib == FIRST) ? SECOND : FIRST;

    *library = dlopen(LIB_NAMES[*current_lib], RTLD_NOW);
    
    if (!(*library)) {
        int len = snprintf(buffer, BUFFER_SIZE, "error switching libs: %s\n", dlerror());
        write(STDERR_FILENO, buffer, len);
        return ER_DLOPEN;
    }

    dlerror();

    *sin_integral_func = (sin_integral_func_t)dlsym(*library, "sin_integral");
    if (!*sin_integral_func) {
        int len = snprintf(buffer, BUFFER_SIZE, "error finding 'sin_integral': %s\n", dlerror());
        write(STDERR_FILENO, buffer, len);
        return ER_DLSYM;
    }

    *sort_func = (sort_func_t)dlsym(*library, "sort");
    if (!*sort_func) {
        int len = snprintf(buffer, BUFFER_SIZE, "error finding 'sort': %s\n", dlerror());
        write(STDERR_FILENO, buffer, len);
        return ER_DLSYM;
    }

    int len = snprintf(buffer, BUFFER_SIZE, "switched to library: %s\n", LIB_NAMES[*current_lib]);
    write(STDOUT_FILENO, buffer, len);

    return OK;
}

void command_1(sin_integral_func_t sin_integral_func) {
    if (!sin_integral_func) return;

    char* arg1 = strtok(NULL, " \t\n");
    char* arg2 = strtok(NULL, " \t\n");
    char* arg3 = strtok(NULL, " \t\n");
    
    char buffer[BUFFER_SIZE];
    int len = 0;

    if (arg1 && arg2 && arg3) {
        float a = atof(arg1);
        float b = atof(arg2);
        float e = atof(arg3);
        float res = sin_integral_func(a, b, e);
        len = snprintf(buffer, BUFFER_SIZE, "sin_integral(%.2f, %.2f, %.4f) result: %.6f\n", 
                      a, b, e, res);
        write(STDOUT_FILENO, buffer, len);
    } else {
        const char msg[] = "error: missing arguments for command 1 (need 3: a b e)\n";
        write(STDERR_FILENO, msg, sizeof(msg)-1);
    }
}

void command_2(sort_func_t sort_func) {
    if (!sort_func) return;

    char* token;
    int array[100];
    size_t count = 0;
    
    while ((token = strtok(NULL, " \t\n")) != NULL && count < 100) {
        array[count++] = atoi(token);
    }
    
    char buffer[BUFFER_SIZE];
    int len = 0;

    if (count > 0) {
        int *res = sort_func(array, count);
        
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
    const char* LIB_NAMES[] = {"./lib1.so", "./lib2.so"};
    int current_lib = FIRST;

    sin_integral_func_t sin_integral_func = NULL;
    sort_func_t sort_func = NULL;
    void* library = NULL;
    char buffer[BUFFER_SIZE];

    library = dlopen(LIB_NAMES[current_lib], RTLD_NOW);
    if (!library) {
        int len = snprintf(buffer, BUFFER_SIZE, "error loading initial lib: %s\n", dlerror());
        write(STDERR_FILENO, buffer, len);
        return ER_DLOPEN;
    }

    sin_integral_func = (sin_integral_func_t)dlsym(library, "sin_integral");
    sort_func = (sort_func_t)dlsym(library, "sort");

    if (!sin_integral_func || !sort_func) {
        const char msg[] = "error: functions not found in library\n";
        write(STDERR_FILENO, msg, sizeof(msg)-1);
        return ER_DLSYM;
    }

    {
        const char *msg = "dynamic\ncommands: 0 - switch lib; 1 - sin_integral a b e; 2 - sort numbers\n> ";
        write(STDOUT_FILENO, msg, strlen(msg));
    }

    int bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        char *token = strtok(buffer, " \t\n");
        if (!token) {
             write(STDOUT_FILENO, "> ", 2);
             continue;
        }

        int cmd = atoi(token);
        switch (cmd) {
            case 0:
                if (command_0(LIB_NAMES, &library, &current_lib, &sin_integral_func, &sort_func) != OK) 
                    return ER_DLOPEN;
                break;
            case 1:
                command_1(sin_integral_func);
                break;
            case 2:
                command_2(sort_func);
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

    if (library) dlclose(library);
    write(STDOUT_FILENO, "\n", 1);
    return OK;
}